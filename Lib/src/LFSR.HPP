/* Linear Feedback Shift Register class definition */

#ifndef LFSRHPP
#define LFSRHPP
#include "gfield.hpp"                  /* C++ Galois Field base class */

/* Standard Definitions */

#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long


/* Linear feedback shift register */

class lfsr: public gfield
{
	uint *syndromes;                    // syndrome lookup table

public:
	operator uint();

   lfsr(uint=0);
   ~lfsr(); 
   int issyndrome();
   uint shiftr(uint);
   uint shiftl(uint);
};

#endif

