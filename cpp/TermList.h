#ifndef TERM_LIST_H_INCLUDED
#define TERM_LIST_H_INCLUDED

#include "CommonHeaders.h"

class Term;

class TermList
{
public:
   void set(int index, Term* term);
   Term*& operator[](int index);

private:
   vector<Term*> _vector;

};

#endif
