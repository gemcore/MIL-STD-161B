/*============================================================================
**
** File:       sertst.cpp
**
** Purpose:    Serial Port test program.
**
**
**
** Date:       November 21, 1990
**
** Author:     Alan D. Graves
**
** Exports:    
**
**
** Rev  Date      Id    Description
** ---  --------- ---   -----------
** 1.0  21-Nov-90 ADG   Created.
**	
**============================================================================
*/

#include <stream.hpp>   /* C++ stream input/output class */
#include	"serial.hpp"   /* Serial port I/O class */


#ifdef DEBUG
int debug(const char *, ...);
#else
#define  debug ;
#endif


/* Local variables */

char dflag=0,  		                  // debug output flag
     qflag=0,					      	   // quiet mode
     lflag=0;					      	   // download

char *ifile=NULL,                      // command line input file
     *ofile=NULL;                      // command line output file

class chan
{
   FILE *ifp;                          // input file pointer
   FILE *ofp;                          // output file pointer
   serial *ser;                        // serial pointer
   char *buffer;                       // input file buffer

public:
   chan(char *,char *);
   ~chan();
   download();
   terminal();
};

chan::chan(char *ifile,char *ofile)
{
   buffer = NULL;                      // input file buffer
   ser = NULL;                         // Serial port access
   ifp = NULL;                         // File access
   ofp = NULL;
	if (ifile != NULL)                  // open required input/output files
	{
		if ((ifp=fopen(ifile,"rb")) == NULL)
		{
         cerr << "Can't open input file " << ifile << "\n";
         return;
		}
	}
	else
   {
		ifp = stdin;
   }
   if ((ofp=fopen(ofile,"wb")) == NULL)
	{
	   ofp = stdout;
	}
   ser = new serial(COM1);             // attach serial port COM1
   buffer = new char[80+1];            // allocate input file buffer
}

chan::~chan() : ()
{
   if (buffer)
      delete buffer;
   if (ser)
      delete ser;
   if (ifp)
      fclose(ifp);
   if (ofp)
      fclose(ofp);
}


chan::download()
{
   cerr << "Downloading... Hit any key to abort\n";

   ser->sputs("\r");                   // get monitor prompt
   ser->sputs("L\r");                  // monitor download command
   while (1)
	{
		int c = ser->sgetchar();
      if (c >= 0)
      {
         switch (c)
         {
         case XOFF:
         case XON:
            break;
         case '\r':
            break;
         default:
            fputc(c,ofp);
         }
         continue;
      }
		if (ser->tx_done)
		{
         if (!fgets(buffer,80,ifp))
         {
            return;
      	}
         debug("buffer=%s",buffer);
			ser->tx_done = 0;
         ser->sputs(buffer);           // output data to serial port
		}
		if (kbhit())
		{
         getch();
			cerr << "aborted!\n";
         break;
		}
	}
}

chan::terminal()
{
   ser->sputs("\r");                   // get monitor prompt
   while (1)
	{
		int c = ser->sgetchar();
      if (c >= 0)
      {
         switch (c)
         {
         case XOFF:
         case XON:
            break;
         case '\r':
            break;
         default:
            fputc(c,ofp);
         }
         continue;
      }
		if (kbhit())                     // poll keyboard
		{
         c = getch();
         switch (c)
         {
         case '/':
            fputc('/',ofp);
            while (!kbhit())
               ;
            c = getch();
            if (c == 'x' || c == 'X')
            {
      			cerr << "exit\n";
               return;
            }
            break;
         case '\r':
            ser->sputchar('\n');
         default:
            fputc(c,ofp);
            ser->sputchar(c);
         }
		}
	}
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

            case 'l':
               lflag++;
               break;

            case 'd':
               dflag++;
               break;

            default:
               cerr << "S -- DDK board interface.\n";
            	cerr << "Usage: s [file] [-o][file] -l\n";
            	cerr << "file   input file\n";
               cerr << "-ofile output file\n";
               cerr << "-l     download to board\n";
               #ifdef DEBUG
               cerr << "-d     output debug to stderr\n";
               #endif
               exit(1);
            }
         }
      }
      else
      {
        	ifile = p;
      }
   }

   chan *f=new chan(ifile,ofile);      // initialize a data channel

   if (lflag)
   {
      f->download();                   // download file
   }
   f->terminal();                      // terminal emulator

   delete f;
}


