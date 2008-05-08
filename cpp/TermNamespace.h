#ifndef TERM_NAMESPACE_H_INCLUDED
#define TERM_NAMESPACE_H_INCLUDED

#include "CommonHeaders.h"

class Term;

class TermNamespace
{
public:
   Term*& operator[](const string& str);

private:
   map<string, Term*> _map;

};

#endif // TERM_NAMESPACE_H_INCLUDED
