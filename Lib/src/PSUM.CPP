/* Power Sum Lookup Table Solution methods (used to calc. BCH error bit locations) */

#include "bch.hpp"      /* C++ BCH FEC base class */

extern unsigned char psum_errtbl[63][64][2];

static int bin_coeff[64]=
{
   63,   5,   4,  10,   3,  15,   9,  29,
    2,  34,  14,  50,   8,  37,  28,  20,
    1,  25,  33,  46,  13,  53,  49,  42,
    7,  17,  36,  39,  27,  55,  19,  57,
    0,  62,  24,  61,  32,  23,  45,  60,
   12,  31,  52,  22,  48,  44,  41,  59,
    6,  11,  16,  30,  35,  51,  38,  21,
   26,  47,  54,  43,  18,  40,  56,  58
};

static int coeff_bin[65]=
{
   0x20,0x10,0x08,0x04,0x02,0x01,0x30,0x18,
   0x0C,0x06,0x03,0x31,0x28,0x14,0x0A,0x05,
   0x32,0x19,0x3C,0x1E,0x0F,0x37,0x2B,0x25,
   0x22,0x11,0x38,0x1C,0x0E,0x07,0x33,0x29,
   0x24,0x12,0x09,0x34,0x1A,0x0D,0x36,0x1B,
   0x3D,0x2E,0x17,0x3B,0x2D,0x26,0x13,0x39,
   0x2C,0x16,0x0B,0x35,0x2A,0x15,0x3A,0x1D,
   0x3E,0x1F,0x3F,0x2F,0x27,0x23,0x21,0x20,
   0x00
};

psum::psum()
{
   /*
   ** Initialize a LFSR for use in ecc calculations.
   */
   reg = new lfsr(GX);                 // linear feeack shift register
}

psum::~psum()
{
   delete reg;
}

void psum::ecc(char *buffer)           // add ecc redundant bits to buffer
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
   buffer[i] = '\0';
}

int psum::fec(char *buffer)            // fec buffer bits
{
   register int i;
   register char s1,s3;

   /*
   ** The power sums are the results from r x M. This is calculated by taking
   ** the sum (XOR) of each element in a particular column with the vector r.
   */
   s1 = 99;

   for (i=0; i <= 63; i++)
   {
      if (buffer[i] == '1')            // sum terms of column which appear in r.
      {
         if (s1 == 99)
         {
            s1 = i;
            s3 = ((3*i)%63);
         }
         else
         {
            s1 = bin_coeff[(coeff_bin[s1]) ^ (coeff_bin[i])];
            s3 = bin_coeff[(coeff_bin[s3]) ^ (coeff_bin[((i*3)%63)])];
         }
      }
   }
   int k=0;
   /*
   ** Calc. error locations from power sums S1 & S3 (via lookup table).
   */
   char e1 = psum_errtbl[s1][s3][0];
   char e2 = psum_errtbl[s1][s3][1];

   debug("S1=%06b S3=%06b locations %2d,%2d ",s1,s3,e1,e2);

   if (e1 != 99)
   {
      buffer[e1] ^= 1;
      k++;
      if (e2 != 99)
      {
         buffer[e2] ^= 1;
         k++;
      }
   }
   return k;
}

