#include <stdio.h>      /* C Standard input/output i/f */
#include <stdarg.h>     /* C Standard variable args macros */

#ifdef _DEBUG
extern char dflag;   			         /* debug output flag */

extern "C"
{
int debug(char *format, ...)             /* formatted printf style trace output */
{
   va_list arg_ptr;
   va_start(arg_ptr, format);
   int num_chars=0;
   char s[512];

	if (dflag)
   {
      va_start(arg_ptr,format);		     /* set arg_ptr to start of arg list */
      num_chars = vsprintf(s,format,arg_ptr);
      fprintf(stderr,s);
   }
   return(num_chars);				     /* return number of chars printed */
}
}

#endif


