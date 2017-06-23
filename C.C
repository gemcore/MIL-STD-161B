/*============================================================================
**
** File:       crc.c
**
** Purpose:    This file contains source for a DOS utility program which 
**             performs CRC calculations. The default polynomial is CRC-16
**             however any polynomial of degree <= 16 may be specified. A
**             table driven approach is used in the calculations.
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
#include <string.h>     /* C string i/f */
#include <dos.h>        /* C DOS i/f (defines struct FIND) */


/* Standard Definitions */

#define PRIVATE static
#define EXPORT /**/
#define IMPORT extern

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


/* Constants */

#define	MAX_FILENAME_LEN	80

/* Forward references */

ulong crcfile(void);
int crc(unsigned int, unsigned int *);

/* Local variables */

PRIVATE unsigned int poly=0b1010000000000001;  /* CRC-16:     1+X2+X15 + X16 */

PRIVATE char xflag=0,						/* hexdecimal */
             iflag=0;						/* input file specified */

PRIVATE char *ifile=NULL,           	/* pointers to command line filenames */
             *ofile=NULL,
             *tfile=NULL;
PRIVATE FILE *ifp,*ofp=NULL,*tfp=NULL;	/* file descriptors */
PRIVATE struct FIND *file;



EXPORT main(argc,argv)
int argc;
char *argv[];
{
   char *p,s[21];
   int i;
   ulong k;

	for (i=1; --argc > 0; i++)
	{
		p = argv[i];
      if (*p == '-')
      {
         while (*++p)
         {
            switch (*p)
            {
               case 'x':
                  xflag++;
                  break;

               case 'o':
	         	   ofile = ++p;
                  p = " ";
                  break;

               default:
                  usage();
            }
         }
      }
      else
      {
         if (!strncmp(p,"poly=",5))
         {
            strncpy(s,p+5,20);
            p = strchr(s,'1')+1;
            strrev(p);
            sscanf(p,"%b",&poly);
         }
         else
         {
     			iflag++;
      		ifile = p;
         }
      }
   }

	if ((ofp = fopen(ofile,"wb")) == NULL)
	{
		ofp = stdout;
   }
	if (iflag)
	{
		if ((file = findfirst(ifile,0)) == NULL)
		{
   		fprintf(stderr,"No files match \"%s\".\n",ifile);
         exit(1);
		}
	}
	else
		ifp = stdin;

   do
   {
      if (iflag)
      {
         if ((ifp = fopen(file->name,"rb")) == NULL)
         {
   	   	fprintf(stderr,"Can't open \"%s\" file for input.\n",file->name);
            exit(1);
         }
      }
      k = crcfile();
      fclose(ifp);
      fprintf(ofp," %-9ld",k);
      if (!iflag)
      {
         fprintf(ofp,"\n");
         break;
      }
      fprintf(ofp," %s\n",file->name);
   }
   while ((file = findnext()) != NULL);
}

PRIVATE usage()
{
	fprintf(stderr,"Usage: CRC [file] [-o][file] poly=[P]\n");
	fprintf(stderr,"\n");
	fprintf(stderr," where,  file  input file (wildcards * ? are allowed)\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"       -ofile  output file\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"       -x      use hexdecimal for CRC\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"       poly=P  polynomial (in binary form)\n");
	fprintf(stderr,"               default is X16+X15+X2+1 = 11000000000000101 (ie. CRC-16)\n");
   exit(1);
}

/* Coefficients hi(n) for CRC-16 code. */

PRIVATE uint h[16]=
{
   0b1111111111111101,
   0b0111111111111110,
   0b1100000000000010,
   0b0110000000000001,
   0b0011000000000000,
   0b0001100000000000,
   0b0000110000000000,
   0b0000011000000000,
   0b0000001100000000,
   0b0000000110000000,
   0b0000000011000000,
   0b0000000001100000,
   0b0000000000110000,
   0b0000000000011000,
   0b0000000000001100,
   0b1111111111111011
};

PRIVATE uint getword()
{
   return((fgetc(ifp)&0xff) + (fgetc(ifp)&0xff)<<8);
}

PRIVATE ulong crcfile()
{
   register uint x,w;
   uint y=0;
	ulong count=0;
   int k;
   char s[21];

	while (1)
	{
      x = getword();
      if (feof(ifp))
	    	break;
      sprintf(s,"%016b",y);
      strrev(s);
      sscanf(s,"%016b",&y);
      w = y^x;
      fprintf(stderr,"x=%04x y=%04x w=%04x",x,y,w);
      y = 0;
      for (k=0; k < 16; k++)
      {
         if (w&1)
         {
            y ^= h[16-k];
         }
         w >>= 1;
      }
      fprintf(stderr," new y=%04x\n",y);
		count++;
	}
   if (xflag)
      fprintf(ofp,"%-4x",y);
   else
      fprintf(ofp,"%-5u",y);
	return(count);
}

