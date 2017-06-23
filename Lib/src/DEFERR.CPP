#include <iostream>                  /* C++ stream input/output class */

using namespace std;

int _allocerr = 0;

void default_error(int n,char *s)
{
    cerr << "(" << n << ") " << s << "\n";
    exit(1);
}
