
#include "TermNamespace.h"
#include "Term.h"

Term*& TermNamespace::operator[](const string& str)
{
   return _map[str];
}
