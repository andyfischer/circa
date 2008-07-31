
#include "common_headers.h"

#include "term_list.h"

int
TermList::count() const
{
    return (int) items.size();
}

void
TermList::append(Term* term)
{
    items.push_back(term);
}

void
TermList::setAt(int index, Term* term)
{
    // Make sure there are enough blank elements in the list
    while (items.size() <= index) {
        items.push_back(NULL);
    }

    items[index] = term;
}

void
TermList::clear()
{
    items.clear();
}

Term* TermList::get(int index) const
{
    if (index >= items.size())
        return NULL;
    return items[index];
}

Term* TermList::operator[](int index) const
{
    return get(index);
}

