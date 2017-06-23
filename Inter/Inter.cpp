/*============================================================================
**
** File:       inter.cpp
**
** Purpose:    This file contains source for a DOS utility program which 
**             simulates the MIL-STD-188-161B interleaving buffer scheme.
**             
**
**
** Date:       September 25, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  25-Sep-90 ADG   Created.
**	
**============================================================================
*/

#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <setjmp.h>     /* C Non-local goto i/f */
#include <iostream>     /* C++ stream input/output class */
using namespace std;
#include "channel.hpp"  /* C++ Data Channel class */

/* Constants */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

#ifdef _DEBUG
extern "C" int debug(const char *, ...);
#else
#define  debug
#endif

#define  W 5
#define  N 63


/* Local variables */
 
char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     eflag=0,						         // encode words
     iflag=0,						         // input file specified
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;

class chan: public channel
{
private:
   char buf[W*N];                      // bit buffer for interleaving storage

public:
   chan(char *,char *);
   ~chan();

   void inter();
   void deinter();
   void encode();
   void decode();
};

chan::chan(char *ifile,char *ofile) : channel(ifile,ofile)
{
}

chan::~chan()
{
}

void chan::inter()
{
   register int n,j;
   int w;

   for (j=0,w=0; w < W; w++)           // clock in bits from buffer
   {
      for (n=0; n < N;)
      {
         buf[w*N+n] = buffer[j++];     // buffer bit in memory
         n++;
      }
   }
   for (j=0,n=0; n < N; n++)           // clock out bits with interleaving
   {
      for (w=0; w < W; w++)
      {
         buffer[j++] = buf[w*N+n];
      }
   }
}

void chan::deinter()
{
   register int n,j;
   int w;

   for (j=0,n=0; n < N; n++)           // clock in bits from buffer
   {
      for (w=0; w < W;)
      {
         buf[w*N+n] = buffer[j++];     // buffer bit in memory
         w++;
      }
   }
   for (j=0,w=0; w < W; w++)           // clock out bits without interleaving
   {
      for (n=0; n < N; n++)
      {
         buffer[j++] = buf[w*N+n];
      }
   }
}

void chan::encode()
{
   source = 2;                         // set data source to file only
   flush();
   int i = 0;
   while (1)
   {
      if (!qflag)
         cerr << "buffer #" << ++i << "\r";
      if (get(N*W) != N*W)
      {
         if (count == 0)
         {
            break;
         }
         debug("\ncount=%d\n",count);
         memset(buffer+count,'0',N*W-count);
         count = N*W;
      }
      inter();
      put();                           // output data
	}
   if (!qflag)
      stats();                         // display statistics
}

void chan::decode()
{
   source = 2;                         // set data source to file only
   flush();
   int i = 0;
   while (1)
   {
      if (!qflag)
         cerr << "buffer #" << ++i << "\r";
      if (get(N*W) != N*W)
      {
         if (count == 0)
         {
            break;
         }
         debug("\ncount=%d\n",count);
         memset(buffer+count,'0',N*W-count);
         count = N*W;
      }
      deinter();
      put();                           // output data
	}
   if (!qflag)
      stats();                         // display statistics
}

void process()
{
	chan *f = new chan(ifile, ofile);      // initialize a chan data channel

	if (eflag)
	{
		f->encode();                     // encode interleave
	}
	else
	{
		f->decode();                     // decode interleave
	}

	delete f;
}


int main(int argc, char*argv[])
{
   char *p;
   int i;

	for (i=1; --argc > 0; i++)
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

            case 'q':
               qflag++;
               break;

            case 'e':
               eflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
	            cerr << "INTER -- buffer interleaving.\n";
	            cerr << "Usage: inter [file] [-o][file] [-e]\n";
	            cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-e     interleave N=63 W=5 (default is deinterleave)\n";
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
