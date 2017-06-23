/*============================================================================
**
** File:       div.c
**
** Purpose:    This file contains source for a DOS utility which implements
**             polynomial long division calculation of the BCH(51,12) error
**             correction code word.
**
**
** Date:       October 10, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
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

/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

                     
char buffer[51+12+1];         /* bit buffer for decoding storage */
char g[]={"1010100111001"};   /* generator polynomial (X12+X10+X8+X5+X4+X3+1) */

char dflag=0;						/* debug output flag */
char iflag=0;						/* input file specified */
char *ifile=NULL,*ofile=NULL;	/* pointers to command line filenames */
FILE *ifp,*ofp=NULL;       	/* file descriptors */



main(argc,argv)
int argc;
char *argv[];
{
   char *p;
   int i;

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

            case 'd':
               dflag++;
               break;

            default:
	            fprintf(stderr,"DIV -- BCH(51,12) polynomial long division results.\n");
	            fprintf(stderr,"Usage: DIV [file] [-o][file]\n");
	            fprintf(stderr,"file   input file\n");
               fprintf(stderr,"-ofile output file (no extension assumed)\n");
               fprintf(stderr,"-d     output debug to stderr\n");
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
   uint data;
   int i,j;
   int count;

   while (1)
	{
      memset(buffer,'0',sizeof(buffer)-1);
      for (count=0,i=0; i < 51;)
      {
  	      data = fgetc(ifp);
         if (feof(ifp))
         {
		      break;
         }
         if (data != '0' && data != '1')
            continue;
         buffer[i] = data;
	      count++;
         i++;
      }
      if (count == 0)
         break;

      if (dflag)
      {
         fprintf(stderr,"%s %s\n",g,buffer);
      }
      for (i=0; i < count; i++)
      {
         if (dflag)
            fprintf(stderr,"            %c",buffer[i]);
         if (buffer[i] == '1')
         {
            for (j=0; j < sizeof(g)-2; j++)
            {
               buffer[i+j+1] = (buffer[i+j+1] ^ g[j+1])+'0';
            }
         }
         if (dflag)
            fprintf(stderr," %s\n",&buffer[i+1]);
      }
      if (dflag)
         fprintf(stderr,"              %s\n",&buffer[i+1]);
      fprintf(ofp,"%s\n",&buffer[i]);
	}
}
