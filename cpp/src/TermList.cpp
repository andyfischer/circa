
#include "Term.h"
#include "TermList.h"


void TermList::set(int index, Term* term)
{
   // Check if we need to add empty elements
   while (index >= _vector.size()) {
      _vector.push_back(NULL);
   }

   _vector[index] = term;
}

Term*& TermList::operator[](int index)
{
   // Check if we need to add empty elements
   while (index >= _vector.size()) {
      _vector.push_back(NULL);
   }
   
   return _vector[index];
}

bool TermList::any_need_update() const
{
    for (size_t i=0; i < _vector.size(); i++) {
        if (_vector[i]->needs_update) {
            return true;
        }
    }
    return false;
}

