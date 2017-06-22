/* Syndrome Table Solution methods (used to calc. BCH error bit locations) */

#include "bch.hpp"      /* C++ BCH FEC base class */


synd::synd()
{
   /*
   ** Initialize a LFSR for use in ecc calculations.
   */
   reg = new lfsr(GX);                 // linear feedback shift register
}

synd::~synd()
{
   delete reg;
}

void synd::ecc(char *buffer)           // add ecc redundant bits to buffer
{
   int i;
   reg->r = 0;                         // calc. parity check bits   
   for (i=0; i < N-12; i++)
   {
      reg->shiftl(buffer[i]-'0');
   }
   uint r = (uint)*reg;
   while (i < N)                       // add parity check bits to buffer
   {
      buffer[i++] = (r&0b100000000000)?'1':'0';
      r <<= 1;
   }
}

int synd::fec(char *buffer)            // fec buffer bits
{
   int i,k=0;
   *reg = (lfsr)0;                     // calc. parity check bits   
   for (i=0; i < N; i++)
   {
      reg->shiftl(buffer[i]-'0');
   }
   for (i=0; i < N; i++)
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

