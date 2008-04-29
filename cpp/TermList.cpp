
#include "TermList.h"


Term*& TermList::operator[](int index)
{
   // Check if we need to add empty elements
   if (index >= _vector.size())
      _vector.assign(_vector.size() - index + 1);
   return &_vector[index];
}

TermList::add_term()
{
   Term new_term;
   _vector.push_back(new_term);
   return _vector[_vector.size() - 1];
}
