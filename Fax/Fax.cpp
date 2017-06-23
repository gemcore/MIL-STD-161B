/*============================================================================
**
** File:       fax.cpp
**
** Purpose:    This file contains source for a DOS utility program which 
**             simulates a FAX machine.
**
**
** Date:       October 10, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
** Imports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  10-Oct-90 ADG   Created.
**	
**============================================================================
*/
#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <setjmp.h>     /* C Non-local goto i/f */
#include <iostream>     /* C++ stream input/output class */
using namespace std;
#include "channel.hpp"  /* C++ Data Channel class */
#include "bch.hpp"      /* C++ BCH(51,12) solution methods class */

/* Local variables */
char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     eflag=0,					      	   // encode words
     fflag=0,					      	   // FEC used
     cflag=0,					      	   // compression used
     sflag=0,					      	   // solution type
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;

volatile int  done=0;                  // global done flag

/* Data sequences */
#define  EOL      "000000000001"
#define  S0       "111100010011010"
#define  S1       "111101011001000"
#define  NOT_S1   "000010100110111"

class chan: public channel
{
private:
   bch *s;                             // solution methods for ecc and fec
   uint block_size;                    // size of get data block
   char *scan_patptr;                  // scan string pattern
   int scan_patpos;                    // scan current position in pattern
   int scan_bufpos;                    // scan buffer position
   int scan_size;                      // scan string length
   int scan_rep;                       // scan string repeat factor
   int scan_match;                     // scan matche count
   int scan_flag;                      // scan output flag
   int scan_sync;                      // scan return value
   int scan_sync1;                     // scan return value (previous)
   ulong scan_bcount;                  // scan fec bit count
   void (chan::*state)();              // processing state machine
   void (chan::*scan_state)();         // scan match processing state
   void scan1();                       // processing states
   //void scan2();
   void decode0();
   void decode1();
   void decode1_0();
   void decode1_1();
   void decode4();
   void decode5();
   void decode5_0();
   void decode2();
   void decode2_0();
   void decode2_1();
   void decode3();
   void decode6();
   void decode7();
   void decode8();
   void decode9();

public:
   chan(char *,char *,char);
   ~chan();

   void ecc();
   void fec();
   void preamble();
   void not_eom();
   void som(int);
   void stuffing(int);
   void eol();
   void rtc();
   void eom();
   void encode();

   int getbit();
   void scan(char *,int,char,void (chan::*)());
   void decode();
};


int ctrl_c_handler(struct INT_DATA *)
{
    ++done;                      // just note that a ^C occurred
    return 1;                    // do not chain old handler
}

chan::chan(char *ifile,char *ofile,char flag) : channel(ifile,ofile)
{
   s = (flag)? (bch *)new synd() : (bch *)new pmat();
}

chan::~chan()
{
   delete s;
}

void chan::ecc()
{
   s->ecc(buffer);                     // add ecc redundant bits to buffer
   count += 12;
}

void chan::fec()
{
   ecount += s->fec(buffer+count-N);   // fec error correction on buffer
   count -= 12;
}

void chan::preamble()
{
   debug("Sending preamble...\n");
   for (int i=0; i < 51; i++)
   {
      fmt("0");
      fmt("1");
   }
}

void chan::not_eom()
{
   debug("Sending ~EOM\n");
   for (int i=0; i < 16; i++)
   {
      fmt(NOT_S1);
   }
}

void chan::som(int x)
{
   debug("Sending SOM(X=%d)\n",x);
   fmt(S1);
   fmt(S0);
   for (int i=0; i < x; i++)
   {
      fmt("1");
   }
   fmt(S0);
   fmt(S1);
}

void chan::stuffing(int n)
{
   debug("Sending stuffing %d\n",n);
   for (int i=0; i < n; i++)
   {
      fmt("1");
   }
}
    
void chan::eol()
{
   debug("Sending EOL\n");
   fmt(EOL);
}

void chan::rtc()
{
   debug("Sending RTC\n");
   for (int i=0; i < 12; i++)
   {
      fmt(EOL);
   }
}

void chan::eom()
{
   debug("Sending EOM\n");
   for (int i=0; i < 16; i++)
   {
      fmt(S1);
   }
}

void chan::encode()
{
#if 0
	printf("syndromes:\n");
	for (int i = 0; i < 63; i++)
	{
		printf(" [%02d]: ", i);
		for (int k = 0; k < 12; k++)
		{
			uint mask = (1 << 11) >> k;
			putchar((s->reg->syndromes[i] & mask) ? '1' : '0');
		}
		putchar('\n');
	}
#endif
   flush();
   source = 1;                         // set data source to queue only
   block_size = N-12;                  // set encode block size
   preamble();                         // preamble of 0|1 for min. 1s (max. of 2s to SOM)
   not_eom();                          // 16 inverted EOMs (or more)
   if (cflag)                          // determine mode
   {
      if (fflag)
      {
         debug("compressed\n");
         som(9);                       // SOM with X value 
         stuffing(0);                  // stuffing of 1 (2s min. to EOL)
         debug("FEC used\n");
         som(255);                     // FEC enabled
      }
      else
      {
         debug("compressed\n");
         som(9);                       // SOM with X value 
         stuffing(0);                  // stuffing of 1 (2s min. to EOL)
         debug("FEC not used\n");
         som(254);                     // FEC disable
      }
      putq();
      stuffing(block_size-12);         // stuffing of 1 (2s min. 3s max. to EOL)
      eol();                           // EOL
      get(block_size);
      if (fflag)
         ecc();                        // error correction code
      put();
      debug("\nFax data\n");
      source = 3;                      // set data source to queue and file
      while (get(block_size) == block_size)
      {
         if (done)
            return;                    // check for ^C
         if (fflag)
            ecc();                     // error correction code
         put();                        // output buffer + ecc
      }
      rtc();                           // RTC
      stuffing(0);                     // as required?
      eom();                           // EOM
      stuffing(51);                    // as required?
      get(block_size-count);
      if (fflag)
         ecc();                        // error correction code
      put();                           // output buffer + ecc
      while (get(block_size) == block_size)
      {
         if (done)
            return;                    // check for ^C
         if (fflag)
            ecc();                     // error correction code
         put();                        // output buffer + ecc
      }
      stuffing(block_size-count+51);   // stuff for even number of 51 bits
      get(block_size-count);
      if (fflag)
         ecc();                        // error correction code
      put();                           // output buffer + ecc
   }
   else
   {
      debug("uncompressed, FEC not used\n");
      som(41);                         // SOM with X value 
      stuffing(0);                     // stuffing of 1 (2s min. to EOL)
      putq();
      stuffing(block_size);            // stuffing of 1 (2s min. 3s max. to EOL)
      putq();
      debug("\nFax data\n");
      source = 3;                      // set data source to queue and file
      get(1728);
      do 
      {
         if (done)
            return;                    // check for ^C
         debug("S0 S0\n");
         fmt(S0);
         fmt(S0);
         putq();
         put();                        // output buffer
      }
      while (get(1728) == 1728);
      stuffing(1728-count);            // stuff to end of line!
      stuffing(0);                     // stuffing of 1 (2s min. to EOM?)
   }
   debug("\n");
   eom();                              // EOM
   putq();
   debug("\nEnd of data\n");

   if (!qflag)
      stats();                         // display statistics
}

int chan::getbit()
{
   if (fflag)
   {
      do
      {
         int i = bcount-scan_bcount;
         debug("\rfec %2-d         ",i);
         if (i >= block_size)
         {
            if (i == block_size)
            {
               fec();                  // forward error correction
            }
            scan_bcount += block_size;
            break;
         }
      }
      while (get(1));                  // get next bit 
      buffer[count] = '\0';
      debug("count=%d [%s]\n",count,buffer);
   }
   else
   {
      get(1);                          // get next bit 
   }
   if (count == 0)
   {
      debug("end of data\n");
      done = 1;
      return 0;
   }
   return 1;
}

void chan::scan(char *s,int r,char flag,void (chan:: *next)())
{
   debug("scan rep=%d '%s'\n",r,s);
   scan_patptr = s;                    // pattern pointer
   scan_patpos = 0;                    // pattern position
   scan_size = strlen(s);              // pattern bit size
   scan_rep = r;                       // repeat factor
   scan_flag = flag;                   // output flag
   scan_sync = 0;                      // sync value 
   scan_match = 0;                     // number of matches
   scan_state = next;                  // scan next processing state
   state = &chan::scan1;               // start scan state-machine
}

void chan::scan1()
{
   if (scan_patpos == scan_size)
   {
      if (++scan_match >= scan_rep)
      {
         debug("\n%-3d %-3d %-3d %-2d match ",count,scan_bufpos,scan_patpos,scan_match);
         scan_sync = bcount-count;
         debug("sync=%d\n",scan_sync);
         shift(scan_rep*scan_size); // flush sync bits
         scan_bufpos = 0;           // reset scan buffer position
         state = scan_state;        // go to next processing state...
         return;
      }
      debug("\n%-3d %-3d %-3d %-2d ",count,scan_bufpos,scan_patpos,scan_match);
      scan_patpos = 0;
   }
   debug("\r%-3d %-3d %-3d %-2d ",count,scan_bufpos,scan_patpos,scan_match);
   if (scan_bufpos == count)
   {
      if (!getbit())
         return;
   }
   if (scan_patptr[scan_patpos++] != buffer[scan_bufpos++])
   {
      int i = 1;
      buffer[count] = '\0';
      if (scan_flag)
      {
         put(i);                       // output data bits
      }
      else
      {
         shift(i);                     // flush data bits
      }
      buffer[count] = '\0';
      debug("[%s] ",buffer);
      scan_bufpos = 0;
      scan_patpos = 0;
      scan_match = 0;
   }
}

void chan::decode0()
{
   debug("Synchronizing...\n");
   fflag = 0;                          // FEC off
   block_size = 0;                     // monitor input bit by bit
   scan_bufpos = 0;                    // reset scan buffer position
   debug("Waiting for ~EOM\n");
   scan(NOT_S1,16,0,&chan::decode1);   // ~EOM marks end of preamble
}

void chan::decode1()
{
   debug("Waiting for SOM1\n");
   scan(S1 S0,1,0,&chan::decode1_0);
}
void chan::decode1_0()
{
   debug("Waiting for SOM2\n");
   scan_sync1 = scan_sync;
   scan(S0 S1,1,0,&chan::decode1_1);
}
void chan::decode1_1()
{
   int x = scan_sync - scan_sync1 - (sizeof(S0 S1)-1);
   switch(x)
   {
   case 9:
      debug("compressed\n");
      state = &chan::decode2;
      break;
   case 41:
      debug("uncompressed\n");
      state = &chan::decode4;
      break;
   default:
      debug("Unknown X=%d value\n",x);
      state = &chan::decode0;
   }
}

void chan::decode4()
{
   debug("FEC not used\n");
   debug("Waiting for S0 S0\n");
   block_size = N-12;               // set decode block size
   count = 0;
   scan(S0 S0,1,0,&chan::decode5);   // S0S0 mark lines of Fax data
}

void chan::decode5()
{
   if (!getbit())
      return;
   if (count >= 1728)
   {
      debug("1728 PELS\n");
      put();
      state = &chan::decode5_0;
   }
}

void chan::decode5_0()
{
   if (!getbit())
      return;
   if (count >= 30)
   {
      if (strncmp(S0 S0,buffer,sizeof(S0 S0)-1)) 
      {
         state = &chan::decode8;
      }
      else
      {
         debug("S0 S0\n");
         count -= 30;
         state = &chan::decode5;
      }
   }
}

void chan::decode2()
{
   debug("Waiting for SOM1\n");
   scan(S1 S0,1,0,&chan::decode2_0);
}
void chan::decode2_0()
{
   debug("Waiting for SOM2\n");
   scan_sync1 = scan_sync;
   scan(S0 S1,1,0,&chan::decode2_1);
}
void chan::decode2_1()
{
   int x = scan_sync - scan_sync1 - (sizeof(S0 S1)-1);
   switch (x)
   {
   case 255:
      debug("FEC used\n");
      block_size = N;                  // set decode block size
      fflag = 1;
      scan_bcount = bcount-count;
      state = &chan::decode3;
      break;
   case 254:
      debug("FEC not used\n");
      block_size = N-12;               // set decode block size
      fflag = 0;
      state = &chan::decode3;
      break;
   default:
      debug("Unknown X=%d value\n",x);
      state = &chan::decode0;
   }
}

void chan::decode3()
{
   debug("Waiting for EOL\n");         // EOL marks Fax data
   scan(EOL,1,0,&chan::decode6);
}

void chan::decode6()
{
   debug("Waiting for RTC\n");         // RTC is 12 x EOLs
   if (fflag)
      scan(EOL,12,1,&chan::decode7);
   else
      scan(EOL,12,1,&chan::decode8);
}

void chan::decode7()
{
   debug("Waiting for EOM1\n");        // EOM is 16 x S1s
   scan(S1,16,0,&chan::decode8);
}
void chan::decode8()
{
   debug("Waiting for EOM2\n");        // EOM is 16 x S1s
   block_size = 0;                     // monitor bit by bit
   fflag = 0;
   scan(S1,16,0,&chan::decode9);
}

void chan::decode9()
{
   debug("End of transmission\n");
   done = 1;
   state = &chan::decode0;
}

void chan::decode()
{
   source = 2;                         // set data source to file only
   flush();
   state = &chan::decode0;             // synchronize decoder
   done = 0;
   while (!done)
   {
      (this->*(state))();              // execute decoder state machine
   }
   if (!qflag)
      stats();                         // display statistics
}

void process(void)
{
	chan *f = new chan(ifile, ofile, sflag);// initialize a data channel

	if (eflag)
	{
		f->encode();                     // encode protocol
	}
	else
	{
		f->decode();                     // decode protocol
	}

	delete f;
}


void main(int argc,char *argv[])
{
   char *p;
   int i;

	for (i=1; --argc > 0; i++)          // parse command line options
	{
		p = argv[i];
		if (*p == '-')
		{
         while (*++p)
         {
            switch(*p)
            {
            case 'o':
      		   ofile = ++p;
               p = " ";
               break;

            case 'e':
               eflag++;
               break;

            case 'f':
               fflag++;
               break;

            case 'c':
               cflag++;
               break;

            case 's':
               sflag++;
               break;

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "FAX -- fax encoder/decoder simulator.\n";
               cerr << "Usage: fax [file] [-o][file] [-efc]\n";
               cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-e     encoder (default is decoder)\n";
               cerr << "-f     FEC used\n";
               cerr << "-c     compression used\n";
               cerr << "-s     solution type parity matrix (default is syndrome)\n";
               cerr << "-q     quiet mode\n";
               #ifdef _DEBUG
               cerr << "-d     output debug to stderr\n";
               #endif
               exit(1);
            }
         }
      }
      else
      {
     		ifile = p;
      }
   }

   process();
}
