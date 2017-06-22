/* Galois Field class definition */

#ifndef GFIELDHPP
#define GFIELDHPP
#include <iostream>                  /* C++ stream input/output class */

using namespace std;

/* Standard Definitions */

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

/* Maximum number of Galois Field elements for G(2^6-1) */

#define N               63             

/* Galois Field elements */

#define GU              0b000000       // GF(undefined)
#define G0              0b100000       // GF(0) = a^0 = 1
#define G1              0b010000       // GF(0) = a^1
#define G2              0b001000       // GF(0) = a^2
#define G3              0b000100       // GF(0) = a^3
#define G4              0b000010       // GF(0) = a^4
#define G5              0b000001       // GF(0) = a^5
#define G6              0b110000       // GF(6) = a^6


/* Galois Field class (fixed length binary polynomial representation) */

class gfield
{
   uint G;                             // G(X) generator polynomial
   void shift(gfield&,int);
public:
   uint r;                             // binary value
   gfield(uint ro=GU,uint gx=G6)       // constructors
      { r = ro; G = gx; }
   gfield(gfield& g)
      { r = g.r; G = g.G; }
	~gfield()   							   // destructor
      {}
   gfield& operator=(gfield&);         // assignment
   gfield& operator=(uint n)
      { r = n; return *this; }
   gfield operator+(gfield&);          // addition (XOR)
   gfield operator+(uint);
   bool operator==(gfield& g)               // equality
      { return r == g.r; }
   bool operator==(uint n)
      { return r == n; }
   bool operator!=(gfield& g)               // inequality
      { return r != g.r; }
   bool operator!=(uint n)
      { return r != n; }
   gfield operator>>(int);             // multiplication by a^n (SHR)
   gfield operator[](int);             // array index
   void decode(char *p)                // ostream value decoding
      { sprintf(p,"%06b",r); }

   friend class lfsr;
};


#endif

