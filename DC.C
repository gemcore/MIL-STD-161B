/*============================================================================
**
** File:       dc.c
**
** Purpose:    This file contains source for a DOS utility program which 
**             converts LANIER fax DUMP data in an output stream of 1 & 0's.
**	
**
**
** Date:       December 27, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  27-Dec-90 ADG   Created.
**	
**============================================================================
*/

#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <ctype.h>      /* C type macros */


/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


/* Local variables */

ulong bcount=0;				   /* count of number of data bits */
uint ncols=0;                 /* columnize output width */
char dflag=0;					   /* debug output */
char qflag=0;					   /* quiet mode */
char *ifile=NULL,*ofile=NULL;	/* pointers to command line filenames */
FILE *ifp,*ofp=NULL;       	/* file descriptors */



main(argc,argv)
int argc;
char *argv[];
{
   char *p;
   int i,j=0;

	for (i=1; --argc > 0; i++)
	{
		p = argv[i];
		if (*p == '-')
		{
         switch(*++p)
         {
            case 'o':
      			ofile = ++p;
               break;

            case 'c':
      			ncols = atoi(++p);
               break;

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               fprintf(stderr,"DC -- Convert binary DUMP data.\n");
	            fprintf(stderr,"Usage: dc [file] [-o][file] [-cnn]\n");
	            fprintf(stderr,"file   input file\n");
               fprintf(stderr,"-ofile output file (must be used with -c option)\n");
               fprintf(stderr,"-cnn   columnize to nn width\n");
               fprintf(stderr,"-q     quiet mode\n");
               fprintf(stderr,"-d     debug output to screen\n");
               exit(1);
         }
      }
      else
      {
     		ifile = p;
      }
   }

   /* Open input and output files */

	if ((ofp = fopen(ofile,"wb")) == NULL)
	{
		ofp = stdout;
	}
	if (ifile != NULL)
	{
		if ((ifp = fopen(ifile,"rb")) == NULL)
		{
			if ((ifp = fopen(ifile,"rb")) == NULL)
			{
				fprintf(stderr,"Can't open \"%s\" file for input.\n",ifile);
            exit(1);
			}
		}
	}
	else
   {
		ifp = stdin;
   }

   process();
}


process()
{
   register uint data;
   register int j;
   int skip=0,i=0;

   while (1)
	{
      data = fgetc(ifp);
      if (feof(ifp))
      {
	      break;
      }
      if (data == '\n' || data == '\r' || data == 0x1a)
      {
         continue;
      }
      data = toupper(data);
      switch(data)
      {
      case '<':
         if (dflag)
         {
            fputc('\n',ofp);
            fputc(data,ofp);
         }
         skip++;
         break;
      case '>':
         if (dflag)
         {
            fputc(data,ofp);
         }
         skip--;
         break;
      default:
         if (!skip)
         {
            if (isdigit(data))
               data -= '0';
            else
               data -= ('A'-10);
            for (j=0; j < 4; j++)
            {
               if (ncols)
               {
                  fputc((data&0x08)?'1':'0',ofp);
                  if (++i == ncols)
                  {
                     fputc('\n',ofp);
                     i = 0;
                  }
               }
               data <<= 1;
            }
            bcount += 4;
         }
         else
         {
            if (dflag)
            {
               fputc(data,ofp);
               if (++i == ncols)
               {
                  fputc('\n',ofp);
                  i = 0;
               }
            }
         }
      }
	}
   fflush(ofp);
   if (!qflag)
      fprintf(stderr,"%ld bits\n",bcount);
}

