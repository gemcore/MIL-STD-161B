/*============================================================================
**
** File:       div.cpp
**
** Purpose:    This file contains source for a DOS utility which implements
**             polynomial long division calculations of error correction code
**             word.
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
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         

char g[80]={"1010100111001"};          // generator polynomial (X12+X10+X8+X5+X4+X3+1)


class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors

public:
   chan(char *,char *);
   ~chan();
   process(int);
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
}

chan::~chan() : ()
{
}

chan::process(int word_size)
{
   int n=strlen(g)-1;
   char *q = new char[word_size+1];

   source = 2;                         // set data source to file only
   flush();
   int i,j;
   while(1)
   {
      memset(buffer,'0',word_size+n);
      buffer[word_size+n] = '\0';
      if (get(word_size) != word_size) // get line of data
         break;
      memset(q,'0',word_size);
      q[word_size] = '\0';
      if (dflag)
      {
         fprintf(stderr,"  %s %s\n",g,buffer);
      }
      for (i=0; i < word_size; i++)
      {
         if (dflag)
         {
            fprintf(stderr,"%c",buffer[i]);
         }
         if (buffer[i] == '1')
         {
            q[i] = '1';
            for (j=0; j < n; j++)
            {
               buffer[i+j+1] = (buffer[i+j+1] ^ g[j+1])+'0';
            }
         }
         if (dflag)
         {
            fprintf(stderr," %s\n",&buffer[i+1]);
         }
      }
      if (dflag)
      {
         fprintf(stderr,"  %s\n",&buffer[i+1]);
      }
      if (dflag)
      {
         fprintf(stderr,"r=%s\nq=%s\n",&buffer[count],q);
      }
      fprintf(ofp,"%s %s\n",&buffer[count],q);
   }
}



main(argc,argv)
int argc;
char *argv[];
{
   char *p;
   int i,word_size=strlen(g)-1;

	for (i=1; --argc > 0; i++)          // parse command line options
	{
		p = argv[i];
		if (*p == '-')
		{
         while (*++p)
         {
            switch(*p)
            {
            case 'n':
      			word_size = atoi(++p);
               p = " ";
               break;

            case 'o':
      			ofile = ++p;
               p = " ";
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "DIV -- Polynomial long division.\n";
            	cerr << "Usage: div [file] [-o][file] [-nN] [poly=P]\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-nN    data word size\n";
	            cerr << "poly=P polynomial (in binary form)\n";
	            cerr << "       defaults N=51 and P=X12+X10+X8+X5+X4+X3+1 (ie. BCH(51,12)\n";
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

   f->process(word_size);              // process data

   delete f;
}


