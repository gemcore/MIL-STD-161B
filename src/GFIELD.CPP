/* Galois Field class methods */

#include "gfield.hpp"                  /* Galois Field class */


void gfield::shift(gfield& g,int n)
{
   for (int i=0; i < n; i++)
   {
      int bit = (g.r & 1);             // calculate next element 
      g.r >>= 1;                       // multiply by a^0
      if (bit)
      {
         g.r ^= G;                     // degree exceeds 2m-1
      }
   }
}

gfield& gfield::operator=(gfield& g)   // assignment
{
   if (this != &g)
   {
      r = g.r;
      G = g.G;
   }
   return *this;
}

gfield gfield::operator+(gfield& g)    // addition (XOR)
{
   gfield loc(*this);
   loc.r ^= g.r;
   return loc;
}

gfield gfield::operator+(uint n)
{
   gfield loc(*this);
   loc.r ^= n;
   return loc;
}

gfield gfield::operator>>(int n)       // multiplication by a^n (SHR)
{
   gfield loc(*this);
   shift(loc,n);
   return loc;   
}

gfield gfield::operator[](int i)       // array index
{
   gfield loc(*(this+i));
   return loc;
}

ostream& operator<<(ostream& s, gfield& g)
{
   char buf[80];

   g.decode(buf);
   return s << buf;
}

