/*============================================================================
**
** File:       scg.cpp
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
#include <stream.hpp>   /* C++ stream output class */
#include <fg.h>         /* C Flash Graphics i/f */
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

public:
   chan(char *,char *);
   ~chan() {};
   encode();
   decode();
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
   if (fg_init_all() == FG_NULL)
   {
      cerr << "Unable to open graphics device.\n";
      exit(1);
   }
   line = 0;
}

chan::encode()
{
   char s[7];
   source = 2;                         // set data source to file only
   flush();
   for (fg_coord_t y=0; y <= fg_displaybox[FG_Y2]; y++)
   {
/*
      fg_box_t box;
	   box[FG_X1] = fg_displaybox[FG_X1];
	   box[FG_Y1] = fg_displaybox[FG_Y2]-fg_charbox[FG_Y2];
	   box[FG_X2] = fg_displaybox[FG_X2];
	   box[FG_Y2] = fg_displaybox[FG_Y2];
   	fg_fillbox(FG_BLACK, FG_MODE_SET, ~0, box);
      fg_puts(FG_WHITE,FG_MODE_XOR,~0,FG_ROT0,
            0,fg_displaybox[FG_Y2]-fg_charbox[FG_Y2],
            itoa(line,s,10),fg_displaybox);
      line++;
*/
      for (fg_coord_t x=0; x <= fg_displaybox[FG_X2]; x++)
      {
         fmt((fg_readdot(x,y) == FG_BLACK)?"1":"0");
      }
      putq();
      while (x++ < 1728)
      {
         fmt("0");
      }
      putq();
   }
   fg_term();
   if (!qflag)
      stats();                         // display statistics
}

chan::decode()
{
   char s[7];
   source = 2;                         // set data source to file only
   flush();
   for (fg_coord_t y=0; y <= fg_displaybox[FG_Y2]; y++)
   {
/*
	   fg_box_t box;
	   box[FG_X1] = fg_displaybox[FG_X1];
	   box[FG_Y1] = fg_displaybox[FG_Y2]-fg_charbox[FG_Y2];
	   box[FG_X2] = fg_displaybox[FG_X2];
	   box[FG_Y2] = fg_displaybox[FG_Y2];
   	fg_fillbox(FG_BLACK, FG_MODE_SET, ~0, box);
      fg_puts(FG_WHITE,FG_MODE_SET,~0,FG_ROT0,
            0,fg_displaybox[FG_Y2]-fg_charbox[FG_Y2],
            itoa(line,s,10),fg_displaybox);
      line++;
*/
      if (!get(1728))
         break;
      for (fg_coord_t x=0; x <= fg_displaybox[FG_X2]; x++)
      {
         fg_drawdot((buffer[x] == '1')?FG_BLACK:FG_WHITE,FG_MODE_SET,~0,x,y);
      }
      count = 0;
   }
   fg_puts(FG_WHITE,FG_MODE_SET,~0,FG_ROT0,
           fg_displaybox[FG_X2]-fg_charbox[FG_X2]*14,fg_displaybox[FG_Y1],
           "<Hit any key>",fg_displaybox);
   while (!kbhit())
      ;
   fg_term();
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
               cerr << "SCG -- graphics screen encoder/decoder.\n";
            	cerr << "Usage: scg [file] [-o][file] [-e]\n";
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