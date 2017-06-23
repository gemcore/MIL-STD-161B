/*============================================================================
**
** File:       z.cpp
**
** Purpose:    This file contains source for a DOS utility which calculates
**             the Z-transform coefficient values of hi(n) for a generator
**             polynomial. These values are used for parallel CRC algorithm.
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
     eflag=0,					      	   // encode words
     *ifile=NULL,                      // pointers to command line filenames
     *ofile=NULL;	         

char g[80]={"1010100111001"};          // generator polynomial (X12+X10+X8+X5+X4+X3+1)


class chan: public channel
{
   channel c_base;                     // need to call channel con/destructors

   lfsr *reg;                          // LFSR (used to calc. fec)
   uint *h;                            // hi(n) coefficient array
   uint x[6];                          // binary values in buffer
   int r;                              // bch width
   int y;                              // resulting bch value
   
   uint getbits(char *,int);
   crcl();
   crcr();
   crcz();
   uint crcbits(char,uint);
   get51();
   get63();
   ecc(char *);
   int fec(char *);

public:
   chan(char *,char *);
   ~chan();
   encode();
   decode();
};

chan::chan(char *ifile,char *ofile) : (ifile,ofile), c_base(ifile,ofile)
{
   char *x = buffer;
   r = strlen(g)-1;
   h = new uint[r];
   x[0] = '1';
   memset(x+1,'0',2*r-1);
   x[2*r] = '\0';
   char *t = new char[r+1];
   t[r] = '\0';
   for (int i=0; i < r; i++)           // generate column vector for hi(n)
   {
      if (x[i] == '1')
      {
         for (int j=0; j < r; j++)
         {
            x[i+j+1] = (x[i+j+1] ^ g[j+1])+'0';
         }
      }
      memcpy(t,&x[i+1],r);
      strrev(t);
      sscanf(t,"%012b",&h[i]);
   }
   delete t;

   if (dflag)
   {
      for (i=0; i < r; i++)
      {
         fprintf(stderr,"hi(%2d)= %012b\n",i+1,h[r-i-1]);
      }
   }

   /*
   ** Initialize a LFSR for use in ecc calculations.
   */
   reg = new lfsr(GX);                 // LFSR with G(x)=X12+X10+X8+X5+X4+X3+1
}

chan::~chan() : ()
{
   delete reg;
   delete h;
}

uint chan::getbits(char *buffer,int k)
{
   register uint x=0;
   while (k-- > 0)                     // get bits from buffer
   {
      x <<= 1;
      x |= *(buffer++)-'0';
   }
   return x;
}

chan::crcz()
{
   uint c;
   y = 0;
   crcbits(1,x[0]);
   crcbits(0,x[1]);
   crcbits(0,x[2]);
   crcbits(0,x[3]);
   crcbits(0,x[4]);
   if (dflag)
   {
      fprintf(stderr,"crcz %012b\n",y);
   }
}

chan::crcl()
{
   uint rl = 0;                        // calc. parity check bits   
   for (int i=0; i < N-12; i++)
   {
      if (dflag && (i == 0 || i == 12 || i == 24 || i == 36 || i == 48))
      {
         fprintf(stderr,"%012b\n",rl);
      }
      uint data = (buffer[i]-'0')?0b100000000000:0;
      int bit = rl & 0b100000000000 ^ data;
      rl <<= 1;
      if (bit)
      {
         rl ^= 0b0010100111001;        // ignore MSB of G(X)! (ie. X12 term)
      }
      rl &= 0b111111111111;
   }
   y = rl;
   while (i < N)                       // add parity check bits to buffer
   {
      buffer[i++] = (rl&0b100000000000)?'1':'0';
      rl <<= 1;
   }
   if (dflag)
   {  
      buffer[i] = '\0';
      fprintf(stderr,"crcl %s\n",buffer+i-12);
   }
}

chan::crcr()
{
   uint rr = 0;                        // calc. parity check bits   
   for (int i=0; i < N-12; i++)
   {
      if (dflag && (i == 0 || i == 12 || i == 24 || i == 36 || i == 48))
      {
         fprintf(stderr,"%012b\n",rr);
      }
      uint data = buffer[i]-'0';
      int bit = (rr & 1) ^ data;
      rr >>= 1;
      if (bit)
      {
         rr ^= 0b100111001010;         // bit reversed ignore MSB of G(X)! (ie. X12 term)
//         rr ^= 0b101010011100;         // ignore LSB of G(X)! (ie. X0 term)
      }
   }
   y = rr;
   while (i < N)
   {
      buffer[i++] = (rr&1) + '0';       // add parity check bits to buffer
      rr >>= 1;
   }
   if (dflag)
   {  
      buffer[i] = '\0';
      fprintf(stderr,"crcr %s\n",buffer+i-12);
   }
}

uint chan::crcbits(char flag,uint x)
{
   uint w;
   if (dflag)
   {
      fprintf(stderr,"y=%012b x=%012b ",y,x);
   }
   if (flag)
   {
      w =((x&0b000000000001) ^ ((y&0b100000000000)? 0b000000000001 : 0)) |
         ((x&0b000000000010) ^ ((y&0b010000000000)? 0b000000000010 : 0)) |
         ((x&0b000000000100) ^ ((y&0b001000000000)? 0b000000000100 : 0)) |
         ((x&0b000000001000) ^ ((y&0b000100000000)? 0b000000001000 : 0)) |
         ((x&0b000000010000) ^ ((y&0b000010000000)? 0b000000010000 : 0)) |
         ((x&0b000000100000) ^ ((y&0b000001000000)? 0b000000100000 : 0)) |
         ((x&0b000001000000) ^ ((y&0b000000100000)? 0b000001000000 : 0)) |
         ((x&0b000010000000) ^ ((y&0b000000010000)? 0b000010000000 : 0)) |
         ((x&0b000100000000) ^ ((y&0b000000001000)? 0b000100000000 : 0)) |
         ((x&0b001000000000) ^ ((y&0b000000000100)? 0b001000000000 : 0)) |
         ((x&0b010000000000) ^ ((y&0b000000000010)? 0b010000000000 : 0)) |
         ((x&0b100000000000) ^ ((y&0b000000000001)? 0b100000000000 : 0));
   }
   else
   {
      w = x;
   }
   if (dflag)
   {
      fprintf(stderr,"w=%012b ",w);
   }
   if (w != 0)
   {
      y =((w&0b000000000001)? h[11]: 0) ^
         ((w&0b000000000010)? h[10]: 0) ^
         ((w&0b000000000100)? h[9] : 0) ^
         ((w&0b000000001000)? h[8] : 0) ^
         ((w&0b000000010000)? h[7] : 0) ^
         ((w&0b000000100000)? h[6] : 0) ^
         ((w&0b000001000000)? h[5] : 0) ^
         ((w&0b000010000000)? h[4] : 0) ^
         ((w&0b000100000000)? h[3] : 0) ^
         ((w&0b001000000000)? h[2] : 0) ^
         ((w&0b010000000000)? h[1] : 0) ^
         ((w&0b100000000000)? h[0] : 0);
   }
   if (dflag)
   {
      fprintf(stderr,"y'=%012b\n",y);
   }
   return y;
}

chan::get51()
{
   x[0] = getbits(&buffer[0], r);
   x[1] = getbits(&buffer[12],r);
   x[2] = getbits(&buffer[24],r);
   x[3] = getbits(&buffer[36],r);
   x[4] = getbits(&buffer[48],3);
}

chan::get63()
{
   get51();
   x[5] = getbits(&buffer[51],r);
}

chan::ecc(char *buffer)
{
   crcz();                             // crc on first 51 bits of data
   int i=count;
   while (i < 63)                      // add parity check bits to buffer
   {
      buffer[i++] = (y&0b100000000000)?'1':'0';
      y <<= 1;
   }
}

int chan::fec(char *buffer)
{
   crcz();
   if (x[5] == y)                      // check for errors in data
   {
      return 0;
   }
   crcbits(0,x[5]);

   int k=0;
   *reg = y;                           // parity check bits   
   for (int i=0; i < N; i++)
   {
      uint err=0;                      // check from syndrome match
      if (reg->issyndrome() < N)
      {
         k++;
         err = 1;                      // error correction required
      }
      reg->shiftl(err);
      buffer[i] ^= err;
   }
   return k;
}

chan::encode()
{
   source = 2;                         // set data source to file only
   flush();
   for (int i=1; get(N-12)==N-12; i++) // input unencoded word
   {
      get51();                         // get first 51 bits of data
      if (!qflag)
         cerr << "block #" << i << "\r";
      crcr();
      stats_start();                   // start statistical timer
      ecc(buffer);                     // add redundant ecc bits
      count += 12;                     // insert ecc value in buffer
      stats_update();                  // update statistical information
      rcount++;
      put();                           // output data + ecc
      if (dflag)
      {
         fprintf(stderr,"\n");
      }
	}
   if (!qflag)
      stats();                         // display statistics
}

chan::decode()
{
   source = 2;                         // set data source to file only
   flush();
   for (int i=1; get(N)==N; i++)       // input encoded word
   {
      get63();                         // get first 63 bits of data
      if (!qflag)
         cerr << "block #" << i << "\r";
      stats_start();                   // start statistical timer
      ecount += fec(buffer+count-N);   // fec channel data errors (T=2)
      count -= 12;                     // remove ecc value from buffer
      stats_update();                  // update statistical information
      rcount++;
   	put();                           // output data
	}
   if (!qflag)
      stats();                         // display statistics
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

            case 'e':
               eflag++;
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
               cerr << "-e     encoder (default is decoder)\n";
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

   if (eflag)
   {
      f->encode();                     // encode data
   }
   else
   {
      f->decode();                     // decode data
   }

   delete f;
}


