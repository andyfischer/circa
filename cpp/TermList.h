#ifndef TERM_LIST_H_INCLUDED
#define TERM_LIST_H_INCLUDED

#include "CommonHeaders.h"

class Term;

class TermList
{
public:
   TermList() {}
   
   TermList(Term* term1)
   {
      _vector.push_back(term1);
   }

   void set(int index, Term* term);
   Term* get(int index);
   Term*& operator[](int index);

   bool any_need_update() const;

private:
   vector<Term*> _vector;

};

#endif
