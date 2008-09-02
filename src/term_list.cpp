
#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "errors.h"
#include "term.h"
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
TermList::appendAll(TermList const& list)
{
    if (&list == this)
        throw errors::InternalError("Circular call");

    for (int i=0; i < list.count(); i++)
        append(list[i]);
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

void TermList::remapPointers(TermMap const& map)
{
    for (int i=0; i < _items.size(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

void TermList_alloc(Term* caller)
{
    caller->value = new TermList();
}

void TermList_dealloc(Term* caller)
{
    delete as_list(caller);
}

} // namespace circa
