
#include "common_headers.h"

#include "term_list.h"
#include "term_map.h"

namespace circa {

int
TermList::count() const
{
    return (int) _items.size();
}

void
TermList::append(Term* term)
{
    _items.push_back(term);
}

void
TermList::setAt(int index, Term* term)
{
    // Make sure there are enough blank elements in the list
    while (_items.size() <= index) {
        _items.push_back(NULL);
    }

    _items[index] = term;
}

void
TermList::clear()
{
    _items.clear();
}

Term* TermList::get(int index) const
{
    if (index >= _items.size())
        return NULL;
    return _items[index];
}

Term* TermList::operator[](int index) const
{
    return get(index);
}

void TermList::remap(TermMap& map)
{
    for (int i=0; i < _items.size(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

} // namespace circa
