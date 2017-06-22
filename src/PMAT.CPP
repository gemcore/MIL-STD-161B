/* Parity Matrix Solution methods (used to calc. BCH error bit locations) */

#include "bch.hpp"      /* C++ BCH FEC base class */


pmat::pmat()
{
   register int i;
   /*
   ** Create parity matrix array.
   */
   M = (gfield *)new char[N*T*sizeof(gfield)];
   for (i=0; i < N; i++)
   {
      M[i+0] = M[i+N] = gfield(G0,G6);
   }

   /*
   ** Create power sum array.
   */
   S = (gfield *)new char[T*sizeof(gfield)];
   for (i=0; i < T; i++)
   {
      S[i] = gfield(G0,G6);
   }

   /*
   ** Expand the parity matrix for the BCH code m=6 and t=2
   ** ie. the matrix is a (2^m - 1) X t matrix of GF(2^6) elements.
   */
   for (i=1; i < N; i++)               // fill remaining rows of matrix
   {
      M[i+0] = M[i-1+0]>>1;
      M[i+N] = M[i-1+N]>>3;
   }

   /*
   ** Generate a lookup table to get power of a^(2m-1) from binary value.
   */
   gf[0] = GU;                         // GF(0) is an undefined value!
   register int j;
   for (j=1; j < N+1; j++)
   {
      /*
      ** Note the first column of parity matrix is the set of all GF(2^m) 
      ** field elements. Thus search through the column and look for the 
      ** matching element, the resulting row indicates the power of a^(2m-1).
      */
      for (i=0; i < N; i++)
      {
         if (M[i+0].r == j)
         {
            break;
         }
      }
      gf[j] = i;
   }

   /*
   ** Initialize a LFSR for use in ecc calculations.
   */
   reg = new lfsr(GX);                 // linear feedback shift register
}

pmat::~pmat()
{
   delete reg;
   delete S;
   delete M;
}

void pmat::ecc(char *buffer)           // add ecc redundant bits to buffer
{
   int i;
   reg->r = 0;                         // calc. parity check bits   
   for (i=0; i < N-12; i++)
   {
      reg->shiftr(buffer[i]-'0');
   }
   uint r = (uint)*reg;
   while (i < N)
   {
      buffer[i++] = (r & 1) + '0';     // add parity check bits to buffer
      r >>= 1;
   }
#if 1
   buffer[i] = '\0';
   printf("%s\n", buffer);
#endif
}

int pmat::fec(char *buffer)            // fec buffer bits
{
   /*
   ** The power sums are the results from r x M. This is calculated by taking
   ** the sum (XOR) of each element in a particular column with the vector r.
   */
   register int i;
   S[0] = S[1] = GU;
   for (i=0; i < N; i++)
   {
      if (buffer[i] == '1')            // sum terms of column which appear in r.
      {
         S[0] = S[0] + M[i+0];
         S[1] = S[1] + M[i+N];
      }
   }
   int k=0;
   if (!(S[0] == GU))                  // S1=0 implies vector r is correct.
   {
      /*
      ** Calc. g1 and g2 from power sums S1 & S3 (via Newton's identities).
      */
      gfield g1 = S[0];                // g1 = S1
      gfield g2 = pow(S[0],2);         // g2 = S1^2
      if (!(S[1] == GU))
      {
         g2 = g2 + div(S[1],S[0]);     // g2 = g2 + S3/S1
      }

      /*
      ** Find roots of Sum(X) equation.
      */
      for (i=0; i < N; i++)
      {
         g1 = g1>>1;
         g2 = g2>>2;
         if ((g1 + g2) == G0)
         {
            buffer[N-(i+1)] ^= 1;
            k++;
         }
      }
   }
   return k;
}

