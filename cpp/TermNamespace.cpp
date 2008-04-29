
#include "TermNamespace.h"


Term*& TermNamespace::operator[](const string& str)
{
   return _map[str];
}
