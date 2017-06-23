/* Linear Feedback Shift Register class methods */

#include "lfsr.hpp"     /* C++ Linear Feedback Shift Register class */


lfsr::operator uint()
{
   return r;
}

lfsr::lfsr(uint gx)
{
   r = 0;
   G = gx;

   /*
   ** Initialize a table of syndromes to use in fec calculateions.
   */
   syndromes = new uint[N];
   for (int i=0; i < N; i++)
   {
      for (int j=0; j < N; j++)
      {
         shiftl((j==i)?1:0);
      }
      uint data = (i>0)?0b100000000000:0;
      syndromes[i] = r ^ data;
      r = 0;
   }
}

lfsr::~lfsr()
{
	delete syndromes;
} 

int lfsr::issyndrome()
{ 
   int i;
   for (i=0; i < N; i++)
   {
      if (r == syndromes[i])           // syndrome match found
      {
         break;
      }
   }
   return i;
}

uint lfsr::shiftr(uint data)
{
   int bit = (r & 1) ^ data;
   r >>= 1;
   if (bit)
   {
      r ^= (G>>1);                     // ignore LSB of G(X)! (ie. X0 term)
   }
   return r;
}

uint lfsr::shiftl(uint data)
{
   data = (data)?0b100000000000:0;
   int bit = r & 0b100000000000 ^ data;
   r <<= 1;
   if (bit)
   {
      r ^= (G&0b111111111111);         // ignore MSB of G(X)! (ie. X12 term)
   }
   r &= 0b111111111111;
   return r;
}


