/*============================================================================
**
** File:       bch.cpp
**
** Purpose:    This file contains source for a DOS utility program which
**             implements a BCH(51,12) encoder/decoder. One of two methods
**             for solving the error correction bit location may be specified.
**
**             0) This method is based on the MIL-STD-188-161B military standard.
**                Which involves a LFSR combined with a 63 entry syndrome lookup
**                table.
**
**             1) This method is based on the following research papers:
**                1. Encoding and Error-Correction Porocedures for the
**                   Bose-Chaudhuri Codes.             (by W. W. Peterson)
**                2. Cyclic Decoding Procedures for
**                   Bose-Chaudhuri-Hocquenghem Codes. (by R.T. Chien)
**
**                Algorithm:
**                1. Generate parity check matrix M.
**                2. Using received vectors r take (r x M) to get S1 & S3 values.
**                3. Solve Newtons identies g1 = S1, g2 = S1^2 + S3/S1
**                4. Solve for roots of Sum(X) = X^2 + g1X^1 + g2X^0 = 0
**
**                   This is done via a cyclic procedure which looks for when
**                   the unit element of GF(2^m) (ie. a^0 = 1) is a root or in
**                   other words X^k = 1 (k = 1,2). The relevant equation is
**                   then Sum(gk) = g1 + g2 = 1 What the procedure amounts to is
**                   take g1 and g2 from step 3. shift g1 by a^1 and shift g2 by
**                   a^2 if the sum (XOR) of the two results is 1 (ie. a^0) then
**                   the root of Sum(X) is related to the number of shifts T by
**                   X=a^(63-T).
**
**             2) This method is based on a paper which appeared in IEEE Micro
**                magazine titled: 'Parallel CRC Generation'. The theory
**                involves simulating the LFSR with a Z-transform solution.
**                The advantage of this method is that any number of bits in
**                the code word (up to the Nth degree of the generator
**                polynomial) can be encoded at one time. For the BCH(51,12)
**                code this means that 4x12 bit & 1x3 bit words are required
**                to encode 51 bits.
**
**             3) This method uses the same power sums S1 & S3 calc. with
**                method 1, however a lookup table is used to determine the
**                error locations.
**
**
** Date:       October 3, 1990
**
** Author:     Alan D. Graves
**
** Exports:
** Imports:    From BCH.LIB library.
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  03-Oct-90 ADG   Created.
**
**============================================================================
*/

#include <stdio.h>  /* C Standard input/output i/f */
#include <stdlib.h> /* C Standard Library i/f */
#include <iostream> /* C++ stream input/output class */
using namespace std;
#include "channel.hpp" /* C++ Data Channel class */
#include "bch.hpp"     /* C++ BCH(51,12) solution methods */

/* Local variables */

char dflag = 0,     // debug output flag
     qflag = 0,     // quiet mode
     eflag = 0,     // encode words
     sflag = 0,     // solution type
    *ifile = NULL,	// pointers to command line filenames
    *ofile = NULL;

class chan : public channel
{
private:
   bch *s;          // solution methods for ecc and fec

 public:
   chan(char *, char *, char);
   ~chan();
   void encode();
   void decode();
};

chan::chan(char *ifile, char *ofile, char flag) : channel(ifile, ofile)
{
   switch (flag)
   {
   case 1:
      s = new pmat();
      break;
   case 2:
      s = new psum();
      break;
   default:
      s = new synd();
   }
}

chan::~chan()
{
   delete s;
}

void chan::encode()
{
   source = 2; // set data source to file only
   flush();
   for (int i = 1; get(N - 12) == N - 12; i++) // input unencoded word
   {
      if (!qflag)
         cerr << "block #" << i << "\r";
      buffer[count] = '\0';
      debug("<%s\n", buffer);
      stats_start();  // start statistical timer
      s->ecc(buffer); // add redundant ecc bits
      count += 12;    // insert ecc value in buffer
      stats_update(); // update statistical information
      buffer[count] = '\0';
      debug(">%s\n", buffer);
      rcount++;
      put(); // output data + ecc
   }
   if (!qflag)
      stats(); // display statistics
}

void chan::decode()
{
   source = 2; // set data source to file only
   flush();
   for (int i = 1; get(N) == N; i++) // input encoded word
   {
      if (!qflag)
         cerr << "block #" << i << "\r";
      buffer[count] = '\0';
      debug("<%s\n", buffer);
      stats_start();                        // start statistical timer
      ecount += s->fec(buffer + count - N); // fec channel data errors (T=2)
      count -= 12;                          // remove ecc value from buffer
      stats_update();                       // update statistical information
      buffer[count] = '\0';
      debug(">%s\n", buffer);
      rcount++;
      put(); // output data
   }
   if (!qflag)
      stats(); // display statistics
}

void process()
{
   chan *f = new chan(ifile, ofile, sflag); // initialize a data channel

   if (eflag)
   {
      f->encode(); // encode compression
   }
   else
   {
      f->decode(); // decode compression
   }

   delete f;
}


int main(int argc, char *argv[])
{
   char *p;
   int i;

   for (i = 1; --argc > 0; i++) // parse command line options
   {
      p = argv[i];
      if (*p == '-')
      {
         while (*++p)
         {
            switch (*p)
            {
            case 'o':
               ofile = ++p;
               p = " ";
               break;

            case 'e':
               eflag++;
               break;

            case 's':
               sflag = atoi(++p);
               p = " ";
               break;

            case 'q':
               qflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "BCH -- BCH(51,12) encoder/decoder.\n";
               cerr << "Usage: bch [file] [-e][-o][file]\n";
               cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-e     encoder (default is decoder)\n";
               cerr << "-sn    solution type is:\n";
               cerr << "        n Encoder  Decoder\n";
               cerr << "        0 LFSR     syndrome table match (default)\n";
               cerr << "        1 LFSR     parity matrix solution\n";
               cerr << "        2 parallel power sum lookup table\n";
               cerr << "-q     quiet mode\n";
#ifdef _DEBUG
               cerr << "-d     output debug to stderr\n";
#endif
               return (1);
            }
         }
      }
      else
      {
         ifile = p;
      }
   }

   process();

   return (0);
}
