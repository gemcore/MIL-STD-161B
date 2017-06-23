/*============================================================================
**
** File:       z.cpp
**
** Purpose:    This file contains source for a DOS utility which calculates
**             the Z-transform coefficient values of hi(n) for a generator
**             polynomial. These values are used for a parallel BCH algorithm.
**
**
** Date:       November 8, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  08-Nov-90 ADG   Created.
**	
**============================================================================
*/

#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include <stream.hpp>   /* C++ stream output class */
#include "channel.hpp"  /* C++ Data Channel class */
#include "bch.hpp"      /* C++ BCH(51,12) solution methods */


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

char g[80]={"1010100111001"};          // generator polynomial (X12+X10+X8+X5+X4+X3+1)


class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors
   char **h;                           // hi(n) coefficient array
   int r;

public:
   chan(char *,char *);
   ~chan();
   process();
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
   char *x = buffer;
   r = strlen(g)-1;
   h = (char **)new uint[r];
   char **hrow = (char **)new uint[r];

   for (int k=0; k < r; k++)
   {
      memset(x,'0',2*r);
      memcpy(x,&g[r-k],k+1);
      x[2*r] = '\0';
      char *q = new char[r+1];
      memset(q,'0',r);
      q[r] = '\0';
      for (int i=0; i < r; i++)
      {
         if (x[i] == '1')
         {
            q[i] = '1';
            for (int j=0; j < r; j++)
            {
               x[i+j+1] = (x[i+j+1] ^ g[j+1])+'0';
            }
         }
      }
      hrow[k] = q;                     // row vector for hi(n)
   }
   for (k=0; k < r; k++)               // convert row to column vectors
   {
      char *t = new char[r+1];
      memset(t,'0',r);
      t[r] = '\0';
      for (int j=0; j < r; j++)
      {
         t[j] = *(hrow[j]+k);
      }
      h[k] = t;
   }
   for (k=0; k < r; k++)               // hi(n) row vectors no longer needed
   {
      delete hrow[k];
   }
   delete hrow;
   if (dflag)
   {
      fprintf(stdout,"/* Parallel generation algorithm coefficients */\n\n");
      fprintf(stdout,"unsigned int hi[%2d]=\n{\n",r);
      for (k=0; k < r; k++)
      {
         fprintf(stdout,"\t0b%s",h[r-k-1]);
         if (k < r-1)
            fprintf(stdout,",\n");
      }
      fprintf(stdout,"\n};");
   }
}

chan::~chan() : ()
{
   for (int k=0; k < r; k++)
   {
      delete h[k];
   }
   delete h;
}

chan::process()
{
   int n;
   char *x = buffer;
   char *y = new char[r+1];
   char *w = new char[r+1];
   x[r] = '\0';
   y[r] = '\0';
   w[r] = '\0';
   memset(y,'0',r);
   source = 2;                         // set data source to file only
   flush();
	while (1)
	{
      memset(x,'0',r);
      if (get(r) != r)
	    	break;
   
      stats_start();                   // start statistical timer
      for (n=0; n < r; n++)
      {
         w[n] = (x[n] ^ y[r-1-n]) + '0';
      }
      for (int k=0; k < r; k++)
      {
         if (w[k] == '1')
         {
            for (n=0; n < r; n++)
            {
               y[n] = (y[n] ^ h[k][n]) + '0';
            }
         }
      }
      stats_update();                  // update statistical information
      rcount++;
      if (dflag)
      {
         fprintf(stderr,"y=%s\n",y);
         fflush(stderr);
      }
      fprintf(ofp,"%s\n",y);
      count = 0;
	}
   if (!qflag)
      stats();
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

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "Z -- Z-transform coefficients hi(n).\n";
            	cerr << "Usage: z [file] [-o][file] [poly=P]\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
	            cerr << "poly=P polynomial (in binary form)\n";
	            cerr << "       defaults P=X12+X10+X8+X5+X4+X3+1 (ie. BCH(51,12)\n";
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
         if (!strncmp(p,"poly=",5))
         {
            strncpy(g,p+5,sizeof(g)-1);
         }
         else
         {
        		ifile = p;
         }
      }
   }

   chan *f=new chan(ifile,ofile);      // initialize a data channel

   f->process();                       // process data

   delete f;
}


