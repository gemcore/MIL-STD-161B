/*============================================================================
**
** File:       m.cpp
**
** Purpose:    This file contains source for a DOS utility program which 
**             implements a '0' and '1' stream monitor.
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
#include <stream.hpp>   /* C++ stream output class */
#include "channel.hpp"  /* C++ Data Channel class */


#ifdef DEBUG
int debug(const char *, ...);
#else
#define  debug ;
#endif


/* Local variables */

char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         

int  width=51;					      	   // width of monitor display


class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors
   int width;

public:
   chan(char *,char *,int);
   ~chan();
   monitor();
};

chan::chan(char *ifile,char *ofile,int w) : (ifile,ofile), c_base(ifile,ofile)
{
   width = w;
}

chan::~chan() : ()
{
}

chan::monitor()
{
   source = 2;                         // set data source to file only
   flush();
   while (get(1) > 0)
   {
      fputc(buffer[0],ofp);
      if (count > width)               // out of synchronization
      {
         shift(1);
      }
      buffer[count] = '\0';
      cerr << "\r" << buffer;
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

            case 'w':
               width = atoi(++p);
               if (width <= 0 || width > 80)
               {
                  width = 51;
               }
               p = " ";
               break;

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "M -- Monitor data.\n";
            	cerr << "Usage: m [file] [-o][file] [-wnn]\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-wnn   width of monitor display (default is 51)\n";
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

   chan *f=new chan(ifile,ofile,width);// initialize a data channel

   f->monitor();                       // monitor data

   delete f;
}


