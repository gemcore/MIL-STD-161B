/*============================================================================
**
** File:       err.c
**
** Purpose:    This file contains source for a DOS utility program which
**             simulates errors in the MIL-STD-188-161B BCH encoder/decoder.
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
//#include "stdafx.h"
#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <string.h>     /* C string i/f */
#include <dos.h>        /* C Standard DOS i/f */


/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


/* Constants */

#define N               63    /* length of code word (2^m)-1 (ie. m = 6) */


/* Local variables */

int ecount = 0;                 /* error counter */
int bcount = 0, bsize = 1;         /* burst count and size */
int bit[3];                   /* bits of data to change */
char qflag = 0,					   /* quiet mode */
rflag = 0,						/* generate random bit errors */
dflag = 0,						/* debug output to screen */
iflag = 0;						/* input file specified */
char *ifile = NULL, *ofile = NULL;	/* pointers to command line filenames */
FILE *ifp, *ofp = stdout;      	/* file descriptors */

void process(void);
void init_bits(void);
void rand_bits(int n);

int main(int argc,char *argv[])
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

			case 'r':
				rflag = atoi(++p);
				break;

			case 'b':
				bcount = atoi(++p);
				if ((p = strchr(p, ',')) != NULL)
				{
					bsize = atoi(++p);
				}
				break;

			case 'e':
				do
				{
					bit[j] = atoi(++p);
					if (bit[j] < 1 || bit[j] > N)
					{
						bit[j] = 0;
						fprintf(stderr, "Invalid bit number specified.\n");
					}
					else
					{
						j++;
					}
				} while (j < 3 && (p = strchr(p, ',')) != NULL);
				if (j > 3)
				{
					fprintf(stderr, "Too many bits specified.\n");
					exit(1);
				}
				break;

			case 'q':
				qflag++;
				break;

			case 'd':
				dflag++;
				break;

			default:
				fprintf(stderr, "ERR -- Bit and burst error simulator.\n");
				fprintf(stderr, "Usage: ERR [file] [-o][file] [-bn]\n");
				fprintf(stderr, "file   input file\n");
				fprintf(stderr, "-ofile output file (no extension assumed)\n");
				fprintf(stderr, "-en    bit number to toggle (1-63)\n");
				fprintf(stderr, "-rn    bit(s) at random\n");
				fprintf(stderr, "-bn,m  burst rate,size\n");
				fprintf(stderr, "-q     quiet mode\n");
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
			fprintf(stderr, "Can't open \"%s\" file for input.\n", ifile);
			exit(1);
		}
	}
	else
	{
		ifp = stdin;
	}

	process();
}


void process(void)
{
	register uint data;
	register int i;
	int j = 0, k = 0;

	init_bits();                        /* initialize random bits seed */

	while (!feof(ifp))
	{
		if (++j > bcount)
		{
			j = 0;
			k = 0;
		}
		for (i = 0; i < N;)                /* read in bits */
		{
			data = fgetc(ifp);            /* input bit */
			if (feof(ifp))
				break;
			if (data != '0' && data != '1')
			{
				fprintf(ofp, "%c", data);
				continue;
			}
			data -= '0';

			/* Toggle the specified bits */

			if (bsize)
			{
				if (i == bit[0] - 1 || i == bit[1] - 1 || i == bit[2] - 1)
				{
					if (j == bcount)
					{
						k = 1;
					}
				}
				if (k != 0 && k <= bsize)
				{
					data ^= 1;
					k++;
					ecount++;
				}
			}
			if (dflag)
			{
				fprintf(stderr, "%b", data);
			}
			fprintf(ofp, "%d", data);       /* output bit (data) */
			i++;
		}
		if (dflag)
		{
			fprintf(stderr, "\n");
		}
		rand_bits(rflag);
	}
	if (!qflag)
		fprintf(stderr, "%d errors caused\n", ecount);
}

void init_bits()
{
	if (rflag)
	{
#if 0
		union REGS regs;
		regs.h.ah = 0x00;
		int86(0x1a, &regs, &regs);
		srand((int)regs.x.dx);
#else
		//Todo: Need to randomize the seed value!
		srand(0);
#endif
		rand_bits(rflag);
	}
}

void rand_bits(int n)
{
	if (n > 0)
	{
		bit[0] = rand() % N + 1;
		if (n > 1)
		{
			bit[1] = rand() % N + 1;
			if (n > 2)
			{
				bit[2] = rand() % N + 1;
			}
		}
	}
}
