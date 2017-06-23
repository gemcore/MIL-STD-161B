/*============================================================================
**
** File:       rl.cpp
**
** Purpose:    This file contains source for a DOS utility program which 
**             implements a run-length encoder/decoder. The codes used are
**             based on CCITT Recommendation T 4 of the Red Book.
**
**
** Date:       October 24, 1990
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
** 1.0  24-Oct-90 ADG   Created.
**	
**============================================================================
*/

#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <iostream> /* C++ stream input/output class */
using namespace std;
#include "..\src\channel.hpp" /* C++ Data Channel class */

#ifdef DEBUG
int debug(const char *, ...);
#else
#define  debug ;
#endif

bool kbhit(void) 
{ 
	return false; 
}

/* Local variables */
char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     eflag=0,					      	   // encode words
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         

/* Data sequences */
#define  EOL      "000000000001"

static char *wrl[91] =               // white run-length data sequences
{
	"00110101",                      // 0
	"000111",                        // 1
	"0111",
	"1000",
	"1011",
	"1100",
	"1110",
	"1111",
	"10011",
	"10100",
	"00111",
	"01000",
	"001000",
	"000011",
	"110100",
	"110101",
	"101010",
	"101011",
	"0100111",
	"0001100",
	"0001000",
	"0010111",
	"0000011",
	"0000100",
	"0101000",
	"0101011",
	"0010011",
	"0100100",
	"0011000",
	"00000010",
	"00000011",
	"00011010",
	"00011011",
	"00010010",
	"00010011",
	"00010100",
	"00010101",
	"00010110",
	"00010111",
	"00101000",
	"00101001",
	"00101010",
	"00101011",
	"00101100",
	"00101101",
	"00000100",
	"00000101",
	"00001010",
	"00001011",
	"01010010",
	"01010011",
	"01010100",
	"01010101",
	"00100100",
	"00100101",
	"01011000",
	"01011001",
	"01011010",
	"01011011",
	"01001010",
	"01001011",
	"00110010",
	"00110011",
	"00110100",                      // 63

	"11011",                         // 64
	"10010",
	"010111",
	"0110111",
	"00110110",
	"00110111",
	"01100100",
	"01100101",
	"01101000",
	"01100111",
	"011001100",
	"011001101",
	"011010010",
	"011010011",
	"011010100",
	"011010101",
	"011010110",
	"011010111",
	"011011000",
	"011011001",
	"011011010",
	"011011011",
	"010011000",
	"010011001",
	"010011010",
	"011000",
	"010011011"                      // 1728
};
static char *brl[91] =               // black run-length data sequences
{
	"0000110111",                    // 0
	"010",                           // 1
	"11",
	"10",
	"011",
	"0011",
	"0010",
	"00011",
	"000101",
	"000100",
	"0000100",
	"0000101",
	"0000111",
	"00000100",
	"00000111",
	"000011000",
	"0000010111",
	"0000011000",
	"0000001000",
	"00001100111",
	"00001101000",
	"00001101100",
	"00000110111",
	"00000101000",
	"00000010111",
	"00000011000",
	"000011001010",
	"000011001011",
	"000011001100",
	"000011001101",
	"000001101000",
	"000001101001",
	"000001101010",
	"000001101011",
	"000011010010",
	"000011010011",
	"000011010100",
	"000011010101",
	"000011010110",
	"000011010111",
	"000001101100",
	"000001101101",
	"000011011010",
	"000011011011",
	"000001010100",
	"000001010101",
	"000001010110",
	"000001010111",
	"000001100100",
	"000001100101",
	"000001010010",
	"000001010011",
	"000000100100",
	"000000110111",
	"000000111000",
	"000000100111",
	"000000101000",
	"000001011000",
	"000001011001",
	"000000101011",
	"000000101100",
	"000001011010",
	"000001100110",
	"000001100111",                  // 63

	"0000001111",                    // 64
	"000011001000",
	"000011001001",
	"000001011011",
	"000000110011",
	"000000110100",
	"000000110101",
	"0000001101100",
	"0000001101101",
	"0000001001010",
	"0000001001011",
	"0000001001100",
	"0000001001101",
	"0000001110010",
	"0000001110011",
	"0000001110100",
	"0000001110101",
	"0000001110110",
	"0000001110111",
	"0000001010010",
	"0000001010011",
	"0000001010100",
	"0000001010101",
	"0000001011010",
	"0000001011011",
	"0000001100100",
	"0000001100101"                  // 1728
};

class rll: public channel
{
   int status;                         // current state

   void put_rl(int,char);
   int get_rl(int);
   void put_bits(int,char);

public:
   rll(char *,char *);
   ~rll() {};
   void encode();
   void decode();
};

rll::rll(char *ifile,char *ofile) : channel(ifile,ofile)
{
   status = 0;
}

void rll::put_rl(int j,char flag)
{
   char **rl = (flag)? brl : wrl;
      
   if (j > 63)
   {
      int k = j/64;
      j %= 64;
      debug("\n%-13s %c %d",rl[63+k],(flag)?'b':'w',64*k);
      fmt(rl[63+k]);
   }
   debug("\n%-13s %c %d",rl[j],(flag)?'b':'w',j);
   fmt(rl[j]);
}

int rll::get_rl(int j)
{
   if (j > 0)
   {
      if (j > 63)
      {
         int k = j-63;
         j = k*64;
      }
   }
   return j;
}

void rll::put_bits(int j,char flag)
{
   static char *bits[2]=
   {
      "0000000000000000000000000000000000000000000000000000000000000000",
      "1111111111111111111111111111111111111111111111111111111111111111"
   };

   char *p = bits[flag];

   if (j > 63)
   {
      int k = j/64;
      j %= 64;
      while (k-- > 0)
      {
         fmt(p);
      }
   }
   if (j > 0)
   {
      fmt(p+64-j);
   }
}

void rll::encode()
{
   debug("Encoding compression...\n");
   source = 2;                         // set data source to file only
   flush();
   uint line;
   int i;
   for (line=0; !kbhit() && get(1728) > 0;)        // get line of data
   {
      debug("\n");
      if (!qflag)
         cerr << "line #" << ++line << "\r";
      char flag=0;                     // start with white run-length
      int wc=0;                        // white count
      int bc=0;                        // black count
      for (i=0; i < count; i++)
      {
         if (flag)
         {
            if (buffer[i] == '1')
            {
               bc++;
            }
            else
            {
               put_rl(bc,1);
               bc = 0;
               flag ^= 1;
               wc = 1;
            }
         }
         else
         {
            if (buffer[i] == '0')
            {
               wc++;
            }
            else
            {
               put_rl(wc,0);
               wc = 0;
               flag ^= 1;
               bc = 1;
            }
         }
      }
      if (bc > 0)
      {
         put_rl(bc,1);
         if (i < 1728)
         {
            wc = 1728-i;
            put_rl(wc,0);
            wc = 0;
         }
      }
      if (wc > 0)
      {
         if (i < 1728)
         {
            wc += 1728-i;
         }
         put_rl(wc,0);
      }
      fmt(EOL);
      debug("\n");
      count = 0;                       // reset buffer count
   }
   putq();
   debug("\nEnd of Data\n");
   if (!qflag)
      stats();                         // display statistics
}

void rll::decode()
{
   debug("Decoding compression...\n");
   source = 2;                         // set data source to file only
   flush();
   uint line=0;
   char sync=0,flag=0;                 // sync on white run-length code
   int i=0,j=0,k=0;
   while (!kbhit() && get(1) > 0)      // get bit of data
   {
      if (count < 2)                   // minimum r-l code word is 2 bits
         continue;
      buffer[count] = '\0';
      if (count > 13)                  // out of synchronization
      {
         if (!qflag)
            cerr << *buffer-'0';
         shift(1);
         sync = 0;                     // clear sync flag
      }
      if (count > 12 & !strncmp(buffer,EOL,12))
      {
         if (i != 1728)
         {
            k = 1728-i;
            if (!qflag)
               cerr << EOL << " line #" << line+1 << " Out of sync! w " << k << "\n";
            put_bits(k,0);
         }
         debug("\r%s  EOL\n",EOL);
         putq();                       // put current line of output
         debug("\n");
         if (!qflag)
            cerr << "line #" << ++line << "\r";
         debug("\n");
         i = 0;
         flag = 0;                     // sync on white run-length code
         sync = 1;                     // set in sync flag
         shift(12);
         continue;
      }
      buffer[count] = '\0';
      debug("\r%-13s ",buffer);
      if (sync)
      {
         k = 0;
         for (j=0; j < 91; j++)
         {
            if (flag)
            {
               if (!strcmp(buffer,brl[j]))
               {
                  k = get_rl(j);
                  debug("b %d\n",k);
                  put_bits(k,1);
                  i += k;
                  count = 0;              // reset buffer count
                  break;
               }
            }
            else
            {
               if (!strcmp(buffer,wrl[j]))
               {
                  k = get_rl(j);
                  debug("w %d\n",k);
                  put_bits(k,0);
                  i += k;
                  count = 0;              // reset buffer count
                  break;
               }
            }
         }
         if (j < 91)
         {
            if (k < 64)                   // if terminating code
            {
               flag ^= 1;
            }
         }
      }
   }
   putq();                             // put last line of output
   debug("\nEnd of Data\n");
   if (!qflag)
      stats();                         // display statistics
}

void process(void)
{
	rll *f = new rll(ifile, ofile);        // initialize a rll data channel

	if (eflag)
	{
		f->encode();                     // encode compression
	}
	else
	{
		f->decode();                     // decode compression
	}

	delete f;
}

int rl(int argc,char *argv[])
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

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "RL -- Run-length encoder/decoder.\n";
            	cerr << "Usage: rl [file] [-o][file] [-e]\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-e     encoder (default is decoder)\n";
               cerr << "-q     quiet mode\n";
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

   process();
}
