/* Data Channel definitions */

#ifndef CHANNELHPP
#define CHANNELHPP
#include <stdio.h>      /* C Standard input/output i/f */
#include <stdlib.h>     /* C Standard Library i/f */
#include "slist.hpp"    /* C++ Toolkit singly linked list class */

//#define SERIAL
#ifdef SERIAL
#include "serial.hpp"   /* Serial port I/O class */
#endif
//#define TIMING                         // Compile timing analysis


/* Constants */

#define MAX_BUFFERSIZE  8192           // max. data buffer size (ie. 8K bytes)


/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long


/* Define data queue type */

struct any
{
    char s[0];
};
typedef struct any* pany;

declare(gqueue,pany);
#if 0
struct panygqueue : slist {
	int queue(pany a) { return slist::append(a); }
	pany dqueue() { return(pany)slist::get(); }
	panygqueue() {}
	panygqueue(pany a) : slist((void *)a) {}
};
#endif

class channel
{
#ifdef SERIAL
	serial *isp;                        // input serial pointer
	serial *osp;                        // output serial pointer
#endif
	FILE *ifp;                          // input file pointer
   FILE *ofp;                          // output file pointer
   int count,ecount,rcount;            // counters
   ulong bcount;                       // bit counter
   ulong tmin,tmax,tacc;               // statistical times
   char buffer[MAX_BUFFERSIZE];        // data buffer
   char source;                        // data source (1=queue,2=file,3=both)
   gqueue(pany)q;                      // data queue
#if 0
   panygqueue q;
#endif
   pany qp;                            // data queue pointer
   int qcount;                         // data queue count
   char *cp;

public:
   char *operator& ()
      { return buffer; }
   char operator() (int i)
      { return buffer[i]; }

   channel(char *,char *);
   ~channel();

   uint getq_bit();
   uint getf_bit();
   int putq();
   int putf();
   int get(int);
   void put(int=0);
   int eob(char=0);
   void fmt(char *);

   void stats_start();
   void stats_pause();
   void stats_update();
   void stats();
   void flush();
   void shift(uint=1);

   friend class fax;
   friend class rll;
   friend class chan;
};

#endif

