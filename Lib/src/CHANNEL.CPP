/* Data Channel methods */

#include <stdio.h>
#include <string.h>
#include "generic.hpp"  /* C++ Toolkit generic declarations definitions */
#include "errn.hpp"     /* C++ Error handler constants */
#include "channel.hpp"  /* C++ Data Channel class */


typedef void (* PFC)(int,char *);
extern void default_error(int,char *);
PFC channel_handler = default_error;

channel::channel(char *ifile,char *ofile)
{
#ifdef SERIAL
   isp = NULL;                         // Serial access
   osp = NULL;
   if (!strcmp("COM1",ifile))
   {
      fprintf(stderr,"Input from COM1\n");
      isp = new serial(COM1);
   }
   else if (!strcmp("COM2",ifile))
   {
      fprintf(stderr,"Input from COM2\n");
      isp = new serial(COM2);
   }
   if (!strcmp("COM1",ofile))
   {
      fprintf(stderr,"Output to COM1\n");
      osp = new serial(COM1);
   }
   else if (!strcmp("COM2",ofile))
   {
      fprintf(stderr,"Output to COM2\n");
      osp = new serial(COM2);
   }
#endif
   ifp = NULL;                         // File access
   ofp = stdout;
	if (ifile != NULL)
	{
#ifdef SERIAL
      if (isp == NULL)
#endif
      {
		   if ((ifp=fopen(ifile,"rb")) == NULL)
		   {
            channel_handler(ENOTOPEN,"channel - can't open input file");
            return;
		   }
      }		
	}
	else
   {
		ifp = stdin;
   }
   if (ifile != NULL)
   {
#ifdef SERIAL
      if (osp == NULL)
#endif
      {
         if (ofile)
		 {
            if ((ofp=fopen(ofile,"wb")) == NULL)
	        {
		       ofp = stdout;
	        }
         }
      }
   }

   rcount = 0;
   ecount = 0;
   count = 0;
   bcount = 0L;
   tmin = 0L;
   tmax = 0L;
   tacc = 0L;
   source = 1;                         // get data from queue only
   cp = NULL;                          // initialize empty queue
   qcount = 0;
}

channel::~channel()
{
#ifdef SERIAL
   if (osp)
      delete osp;
   if (isp)
      delete isp;
#endif
   if (ofp)
      fclose(ofp);
   if (ifp)
      fclose(ifp);
}

uint channel::getf_bit()
{
   uint data=EOF;
#ifdef SERIAL
   if (isp)
   {
      int c;
      while (1)
      {
         fprintf(stderr,".\b");
         c = isp->sgetchar();
         if (c < 0)
            continue;
         if (c == XOFF || c == XON || (c != '0' && c != '1'))
            continue;
         if (c == 0x1a)
         {
            c = EOF;
            break;
         }
      }
      bcount++;
      data = c;
   }
   else
#endif
   {
      if (!feof(ifp))
      {
         do
         {
            data = fgetc(ifp);
            if (feof(ifp))
            {
	            return(EOF);
            }
         }
         while (data != '0' && data != '1');
         bcount++;
      }
   }
   return data;
}

uint channel::getq_bit()
{
   uint data=EOF;
   if (cp != NULL)
   {
      do
      {
         data = *cp++;
         if (data == '\0')
         {
            if (qcount <= 0)
            {
               cp = NULL;
               return(EOF);
            }
			qp = q.dequeue();
            cp = qp->s;
            qcount--;
            continue;
         }
      }
      while (data != '0' && data != '1');
      bcount++;
   }
   return data;
}

int channel::putq()                    // put queue data to output file
{
   int i=0;
   while (1)
   {
      uint data = getq_bit();
      if (data == EOF)
      {
         break;
      }
      fputc(data,ofp);
      i++;
   }
   return i;
}

int channel::putf()                    // put file data to output file
{
   int i=0;
   while (1)
   {
      uint data = getf_bit();
      if (data == EOF)
      {
         break;
      }
      fputc(data,ofp);
      i++;
   }
   return i;
}

int channel::get(int n)                // get buffer from source
{
   uint data;
   int i=0;
   if (source&1)
   {
      while (i < n)
      {
         data = getq_bit();
         if (data == EOF)
         {
            break;
         }
         if (count == MAX_BUFFERSIZE)
         {
            channel_handler(ENOMEM,"channel - get buffer overflow");
            break;
         }
         buffer[count++] = data;
         i++;
      }
   }
   if (source&2)
   {
      while (i < n)
      {
         data = getf_bit();
         if (data == EOF)
         {
            break;
         }
         if (count == MAX_BUFFERSIZE)
         {
            channel_handler(ENOMEM,"channel - get buffer overflow");
            break;
         }
         buffer[count++] = data;
         i++;
      }
   }
   return i;
}

void channel::put(int n)
{
   if (n == 0)
   {
      n = count;
   }
   if (n > count)
   {
      channel_handler(EOUTBOUND,"channel - put out of bounds");
   }
   for (int i=0; i < n; i++)           // put buffer data
   {
#ifdef SERIAL
      if (osp)
      {
//   		while (!osp->tx_done)
//            ;
//   	   osp->tx_done = 0;
         osp->sputchar(buffer[i]);     // output data to serial port
      }
      else
#endif
      {
     	   fputc(buffer[i],ofp);
      }
   }
   if (count-n > 0)
   {
      memmove(&buffer[0],&buffer[n],count-n);
   }
   count -= n;
   fputc('\n',ofp);
}

int channel::eob(char src)                 // test for end of data on source
{
   if (src == 0)
   {
      src = source;                    // default to current source
   }
   return (src&1 && cp == NULL) || (src&2 && feof(ifp));
}

void channel::fmt(char *p)
{
   if (cp == NULL)
   {
      cp = "";                         // queue is not empty now
   }
   q.queue((pany)p);                   // add data to queue
   qcount++;
}

void channel::flush()
{
   fflush(ofp);                        // flush output stream

   if (source&1)
   {
      while (getq_bit() != EOF)        // flush queue
         ;
   }
   if (source&2)
   {
      count = 0;                       // flush buffer
   }
}

#ifdef TIMING
ulong timer_count=0;                   // clear timer paused count
char timer_active=0;                   // set timer not paused
#endif

void channel::stats_start()
{
   #ifdef TIMING
   timer_active = 1;
   timer_start();                      // start timer
   #endif
}

void channel::stats_pause()
{
   #ifdef TIMING
   if (timer_active)
   {
      uint s,ms,us;
      timer_read(&s,&ms,&us);          // stop and read timer
      timer_count += s*1000000L+ms*1000L+us;   // total time in micro seconds
      timer_active = 0;
   }
   #endif
}

void channel::stats_update()
{
   #ifdef TIMING
   ulong t = timer_count;              // total time in micro seconds
   if (timer_active)
   {
      uint s,ms,us;
      timer_read(&s,&ms,&us);          // stop and read timer
      t += s*1000000L+ms*1000L+us;     // total time in micro seconds
   }
   timer_count = 0;                    // clear timer paused count
   tacc += t;                          // update accumulated time total
   if (t < tmin || tmin == 0)          // update minimum time
      tmin = t;
   if (t > tmax)                       // update maximum time
      tmax = t;
   if (tmax < tmin)
      tmax = tmin;
   #endif
}

void channel::stats()
{
   flush();
   if (ecount > 0)
   {
      fprintf(stderr,"%d errors corrected\n",ecount);
   }
   fprintf(stderr,"Processed %ld bits",bcount);
   if (rcount > 0)
   {
      #ifdef TIMING
      fprintf(stderr,", %d blocks in %ldus, (%ld %ld %ld us/vector)",
      rcount,tacc,tmin,tacc/rcount,tmax);
      #endif
   }
   fprintf(stderr,".\n");
}

void channel::shift(uint n)                 // shift buffer right by 1
{
   if (n > count)
   {
      channel_handler(EOUTBOUND,"channel - shift out of bounds");
      count = 0;
   }
   else
   {
      memmove(&buffer[0],&buffer[n],count-n);
      count -= n;
   }
}

PFC set_channel_handler(PFC handler)
{
    PFC local = channel_handler;
    channel_handler = handler;
    return local;
}

