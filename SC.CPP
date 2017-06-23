/*============================================================================
**
** File:       sc.cpp
**
** Purpose:    This file contains source for a DOS utility program which 
**             either captures the current  display and writes it to a file
**             as a series of '0's and '1's or reads a file and outputs it to
**             the screen for viewing. The format used is compatible with the
**             BCH utility programs.
**
**
** Date:       November 1, 1990
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
** 1.0  01-Nov-90 ADG   Created.
**	
**============================================================================
*/

#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <disp.h>       /* C display i/f */
#include <stream.hpp>   /* C++ stream output class */
#include "channel.hpp"  /* C++ Data Channel class */


/* Local variables */

char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     eflag=0,					      	   // encode words
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         



class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors
   uint line;                          // current screen line
   int bufsize;
   uint *linebuf;
   char *chrtbl[256];

   getline();
   putline();
   getscreen();
   putscreen();

public:
   chan(char *,char *);
   ~chan();
   encode();
   decode();
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
   disp_open();
   bufsize = disp_numcols;
   linebuf = new unsigned[bufsize];
   line = 0;
   for (int i=0; i < 256; i++)
   {
      chrtbl[i] = new char[9];
      sprintf(chrtbl[i],"%08b",i);
   }
}

chan::~chan() : ()
{
   for (int i=0; i < 256; i++)
   {
      delete chrtbl[i];
   }
   delete linebuf;
   disp_close();
}

chan::putline()
{
   putq();
}

chan::getline()
{
   get(8*disp_numcols);
}

chan::getscreen()
{
   disp_peekbox(linebuf,line,0,line,disp_numcols-1);
   for (int i=0; i < bufsize; i++)
   {
      if (linebuf[i] == 0x0700+' ')
      {
         fmt("00000000");
      }
      else
      {
         fmt(chrtbl[linebuf[i]&0xff]);
      }
   }
}

chan::putscreen()
{
   for (int i=0; i < bufsize; i++)
   {
      if (!strncmp(buffer[i*8],"00000000"))
      {
         linebuf[i] = 0x0700+' ';
      }
      else
      {
         char s[9];
         memcpy(s,&buffer[i*8],8);
         s[8] = '\0';
         uint ch;
         sscanf(s,"%b",&ch);
         linebuf[i] = 0x0700+(ch&0xff);
      }
   }
   count = 0;
   disp_pokebox(linebuf,line,0,line,disp_numcols-1);
}

chan::encode()
{
   source = 2;                         // set data source to file only
   flush();
   do
   {
      getscreen();
      putline();
   }
   while (++line < disp_numrows);
   putq();
   if (!qflag)
      stats();                         // display statistics
}

chan::decode()
{
   source = 2;                         // set data source to file only
   flush();
   do
   {
      getline();
      putscreen();
   }
   while (++line < disp_numrows);
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

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "SC -- screen encoder/decoder.\n";
            	cerr << "Usage: sc [file] [-o][file] [-e]\n";
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

process()
{
   chan *f=new chan(ifile,ofile);      // initialize a data channel

   if (eflag)
   {
      f->encode();                     // encode screen to file
   }
   else
   {
      f->decode();                     // decode file to screen
   }

   delete f;
}


