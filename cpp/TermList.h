#ifndef TERM_LIST_H_INCLUDED
#define TERM_LIST_H_INCLUDED

#include "CommonHeaders.h"

class Term;

class TermList
{
public:
   Term*& operator[](int index);

   Term& addTerm();

private:
   vector<Term*> _vector;

};

#endif
