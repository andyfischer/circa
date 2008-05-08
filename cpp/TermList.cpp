
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

Term* TermList::get(int index)
{
    // Check if we need to add empty elements
    while (index >= _vector.size())
        _vector.push_back(NULL);
    
    return _vector[index];
}

Term*& TermList::operator[](int index)
{
    // Check if we need to add empty elements
    while (index >= _vector.size())
        _vector.push_back(NULL);
   
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

namespace term_list {

#define LIST_DATA(term) (reinterpret_cast<TermList*>((term)->data))

void initialize_data(Term* term)
{
    term->data = new TermList;
}

void set(Term* list, int index, Term* element)
{
    LIST_DATA(list)->set(index, element);
}

Term* get(Term* list, int index)
{
    return LIST_DATA(list)->get(index);
}

} // namespace term_list

