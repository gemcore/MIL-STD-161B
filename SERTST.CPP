/*============================================================================
**
** File:       sertst.cpp
**
** Purpose:    Serial Port test program.
**
**
**
** Date:       November 21, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  21-Nov-90 ADG   Created.
**	
**============================================================================
*/

#include <stream.hpp>   /* C++ stream input/output class */
#include "channel.hpp"  /* C++ Data Channel class */
#include	"serial.hpp"   /* Serial port I/O class */

#define  N     63

#ifdef DEBUG
int debug(const char *, ...);
#else
#define  debug ;
#endif

/* Data sequences */

#define  EOL      "000000000001"
#define  S0       "111100010011010"
#define  S1       "111101011001000"
#define  NOT_S1   "000010100110111"

/* Local variables */

char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     eflag=0,					      	   // encode words
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         


class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors
   serial *ser1;                       // serial port access
   int (*state)();                    // processing state machine
   int (*next_state)();               // next processing state when scanning
   uint block_size;                    // size of get data block
   char *scan_pat;                     // scan string pattern
   int scan_size;                      // scan string length
   int scan_rep;                       // scan string repeat factor
   int scan_flag;                      // scan sync flag
   int scan_sync;                      // scan sync return value
   int scan1();
   int scan2();
   int decode0();
   int decode1();
   int decode2();

public:
   chan(char *,char *);
   ~chan();
   int sync(char *,int,int,char);
   int scan(char *,int,char,int (*)());
   encode();
   decode();
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
   ser1 = new serial(COM1);            // Serial port COM1 access
}

chan::~chan() : ()
{
   delete ser1;
}

int chan::sync(char *s,int n,int r,char flag)
{                                      // synchronize buffer with bit sequence
   char *p=buffer;
   char *q=p;
   for (int i=0; p <= buffer+count-n && i < r;)
   {
      if (!strncmp(s,p,n))
      {
         if (i == 0)
         {
            q = p;                     // save position of 1st match
         }
         i++;                          // count matches
         p += n;                       // skip right over n
      }
      else
      {
         q = p;
         i = 0;                        // non-matching bit
         p += 1;                       // skip right over 1
      }
   }
   while (q-- > buffer)                // output data bits to sync sequence
   {
      if (flag)
      {
         fputc(buffer[0],ofp);
      }
      shift(1);
   }
   if (i < r)
   {
      return 0;
   }
   shift(i*n);                         // ignore data bits of sync sequence
   return(i);
}

int chan::scan(char *s,int r,char flag,int (*next)())
{
   debug("Waiting for %d pattern of '%s'\n",r,s);
   scan_pat = s;
   scan_size = strlen(s);
   scan_rep = r;
   scan_flag = flag;
   scan_sync = 0;
   next_state = next;
   state = _chan_scan1;                // start scan state-machine
}

int chan::scan1()
{
   if (count >= scan_size*scan_rep+block_size)
   {
/*
      if (eflag)
         ecc();                        // error correction code
      else
         fec();                        // forward error correction
*/
      state = _chan_scan2;
   }
}

int chan::scan2()
{
   if ((scan_sync=sync(scan_pat,scan_size,scan_rep,scan_flag)) != 0)
   {
      state = next_state;              // go to next processing state...
   }
}

chan::encode()
{
   source = 2;                         // set data source to file only
   flush();

   while (1)
	{
		if (ser1->tx_done)
		{
         if (!get(1))
         {
            break;
      	}
         if (count == N-12)
         {
			   ser1->tx_done = 0;
            rcount++;
            bcount += count;
            buffer[count] = '\0';
            debug("\t\t%s\r",buffer);
            stats_start();             // start statistical timer
            ser1->sputs(buffer);       // output data to serial port
            stats_update();            // update statistical information
            count = 0;
         }
		}
		if (kbhit())
		{
			cerr << "aborted!\n";
         break;
		}
	}
   if (!qflag)
      stats();                         // display statistics
}

int chan::decode0()
{
   scan("1111",1,0,_chan_decode1);
}

int chan::decode1()
{
   scan(S1 S0,1,0,_chan_decode2);
}

int chan::decode2()
{
   scan(S1 S0,1,0,_chan_decode0);
}

chan::decode()
{
   source = 2;                         // set data source to file only
   flush();
   block_size = 1;                     // monitor bit by bit
   state = _chan_decode0;
   decode0();
   while (1)
	{
		int c = ser1->sgetchar();
      if (c >= 0 && c != XOFF && c != XON && (c == '0' || c == '1')) 
		{
         bcount++;
         buffer[count++] = c;
         buffer[count] = '\0';
         debug("\r%s ",buffer);
         (state)(this);                   // execute decoder state machine
      }

		if (kbhit())                     // poll keyboard
		{
         getch();
			cerr << "aborted!\n";
         break;
		}
	}
   if (!qflag)
      stats();                         // display statistics
}



main(argc,argv)
int argc;
char *argv[];
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

            case 'd':
               dflag++;
               break;

            default:
               cerr << "SERTST -- Serial Port test.\n";
            	cerr << "Usage: sertst [file] [-o][file]\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-e     encoder (default is decoder)\n";
               #ifdef DEBUG
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

   chan *f=new chan(ifile,ofile);      // initialize a data channel

   if (eflag)
   {
      f->encode();                     // encode data
   }
   else
   {
      f->decode();                     // decode data
   }

   delete f;
}


