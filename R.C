/*============================================================================
**
** File:       r.c
**
** Purpose:    This file contains source for a DOS utility program which 
**             generates a stream of 1 & 0's formatted 51 per line.
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
#include <dos.h>        /* C Standard DOS i/f */


/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


/* Constants */

#define N               63    /* length of code word (2^m)-1 (ie. m = 6) */


/* Local variables */

int bit[3];                   /* bits of data to change */
uint nlines=0;						/* generate n x 51-bit lines of 1 & 0's */
char iflag=0;						/* input file specified */
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

            case 'n':
               sscanf(++p,"%u",&nlines);
               break;

            default:
               fprintf(stderr,"R -- Generates a random stream of 1 & 0's formatted 51 per line.\n");
	            fprintf(stderr,"Usage: R [file] [-o][file] [-bn]\n");
	            fprintf(stderr,"file   input file\n");
               fprintf(stderr,"-ofile output file (no extension assumed)\n");
               fprintf(stderr,"-nn    number of lines of 51 bits \n");
               exit(1);
         }
      }
      else
      {
  			iflag++;
     		ifile = p;
      }
   }

   /* Open input and output files */

	if ((ofp = fopen(ofile,"wb")) == NULL)
	{
		ofp = stdout;
	}
	if (iflag)
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
   register int i;

   init_bits();                        /* initialize random bits seed */

   while (nlines--)
	{
      for (i=0; i < 51; i++)
      {
  	      data = (rand()&1)+'0';        /* random bit */
         fprintf(ofp,"%c",data);       /* output bit (data) */
      }
      fprintf(ofp,"\r\n");             /* output new line */
	}
}

init_bits()
{
   union REGS regs;

   if (nlines)
   {
      regs.h.ah = 0x00;
      int86(0x1a,&regs,&regs);
      srand((int)regs.x.dx);;
   }
}
