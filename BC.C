/*============================================================================
**
** File:       bc.c
**
** Purpose:    This file contains source for a DOS utility program which 
**             counts the number of 1 & 0's in the input stream.
**	
**
**
** Date:       October 24, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  24-Oct-90 ADG   Created.
**	
**============================================================================
*/
#include <stdio.h>  /* C Standard input/output i/f */
#include <stdlib.h> /* C Standard Library i/f */

/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

/* Local variables */

ulong bcount = 0;                  /* count of number of data bits */
uint ncols = 0;                    /* columnize output width */
char qflag = 0;                    /* quiet mode */
char *ifile = NULL, *ofile = NULL; /* pointers to command line filenames */
FILE *ifp, *ofp = NULL;            /* file descriptors */

int bc(int argc, char *argv[])
{
   char *p;
   int i, j = 0;

   for (i = 1; --argc > 0; i++)
   {
      p = argv[i];
      if (*p == '-')
      {
         switch (*++p)
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

         default:
            fprintf(stderr, "BC -- Counts the number of data bits.\n");
            fprintf(stderr, "Usage: BC [file] [-o][file] [-cnn]\n");
            fprintf(stderr, "file   input file\n");
            fprintf(stderr, "-ofile output file (must be used with -c option)\n");
            fprintf(stderr, "-cnn   columnize to nn width\n");
            fprintf(stderr, "-q     quiet mode\n");
            exit(1);
         }
      }
      else
      {
         ifile = p;
      }
   }

   /* Open input and output files */

   ofp = stdout;
   if (ofile != NULL)
   {
      if ((ofp = fopen(ofile, "wb")) == NULL)
      {
         ofp = stdout;
      }
   }
   if (ifile != NULL)
   {
      if ((ifp = fopen(ifile, "rb")) == NULL)
      {
         if ((ifp = fopen(ifile, "rb")) == NULL)
         {
            fprintf(stderr, "Can't open \"%s\" file for input.\n", ifile);
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
   register int i = 0;
   while (1)
   {
      data = fgetc(ifp);
      if (feof(ifp))
      {
         break;
      }
      if (data == '0' || data == '1')
      {
         bcount++;
         if (ncols)
         {
            fputc(data, ofp);
            if (++i == ncols)
            {
               fputc('\n', ofp);
               i = 0;
            }
         }
      }
   }
   fflush(ofp);
   if (!qflag)
      fprintf(stderr, "%ld bits\n", bcount);
}
