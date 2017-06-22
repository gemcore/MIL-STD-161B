/* BCH(51,12) Solution definitions (calculates BCH error bit locations) */

#ifndef BCHHPP
#define BCHHPP
#include "gfield.hpp"   /* C++ Galois Field base class */
#include "lfsr.hpp"     /* C++ Linear Feedback Shift Register class */

//#define  PORTABLE

/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

#ifdef _DEBUG
extern "C" int debug(const char *, ...);
#else
#define  debug
#endif

/* Constants */

#define T               2              // number of bits of error correction

/* Generator polynomial G(X) for the BCH(63,51) code. Note the GF(x) elements
   are all polynomials for degree 2m-1 or less that are multiples of G(X). */

#define GX        0b1010100111001      // G(x) = X12+X10+X8+X5+X4+X3+1


/* BCH solution base class */

class bch
{
public:
   lfsr *reg;                          // LFSR (used to calc. ecc)
	
   virtual void ecc(char *buffer)
   { }
   virtual int fec(char *buffer)
   { return 0; }

   friend class synd;
   friend class pmat;
   friend class psum;
};


/* Syndrome Table Solution class definitions */

class synd: public bch
{
public:
   synd();
   ~synd();
   void ecc(char *);
   int fec(char *);
};


/* Parity Matrix Solution class definitions */

class pmat: public bch
{
   gfield *M;                          // pointer to parity matrix
   gfield *S;                          // power sums
   int gf[N+1];                        // GF(x) element to a^n lookup table

   int GF(gfield& x)                   // get unique GF(2^m-1) field element for x
   {
      return gf[x.r];
   }

   int GF(uint x)
   {
      return gf[x];
   }

   gfield pow(gfield& x,int n)         // calc. x^n
   {
      if (n == 0)
      {
         return gfield(G0);
      }
      int i = GF(x)*n;                 
      while (i > N-1)
      {
         i = i-N;
      }
      return gfield(M[i+0]);
   }

   gfield div(gfield& x,gfield& y)     // calc. x/y
   {
      if (x.r == GU)
      {
         return gfield(GU);
      }
      int i = GF(x)-GF(y);
      if (i < 0)
      {
         i += N;
      }
      return gfield(M[i+0]);
   }

public:
   pmat();
   ~pmat();
   void ecc(char *);
   int fec(char *);
};

/* Power Sum Lookup Table Solution class definitions */

class psum: public bch
{
public:
   psum();
   ~psum();
   void ecc(char *);
   int fec(char *);
};

#endif
