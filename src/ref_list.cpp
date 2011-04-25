// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <algorithm>

#include "common_headers.h"

#include "references.h"
#include "term.h"

#include "ref_list.h"

namespace circa {

const size_t MAX_REF_LIST_LENGTH = 10000000;

void RefList::fromTermList(TermList const tl)
{
    resize(tl.length());
    for (int i=0; i < tl.length(); i++)
        setAt(i, tl[i]);
}

void RefList::append(Term* term)
{
    ca_assert(_items.size() < MAX_REF_LIST_LENGTH);
    _items.push_back(term);
}

void RefList::prepend(Term* term)
{
    ca_assert(_items.size() < MAX_REF_LIST_LENGTH);
    _items.insert(_items.begin(), term);
}

void RefList::insert(int index, Term* term)
{
    _items.insert(_items.begin()+index, term);
}

void RefList::appendUnique(Term* term)
{
    int len = length();
    for (int i=0; i < len; i++)
        if (get(i) == term)
            return;

    append(term);
}

int RefList::length() const
{
    ca_assert(_items.size() < MAX_REF_LIST_LENGTH);
    return (int) _items.size();
}

Term* RefList::get(unsigned int index) const
{
    ca_assert(index < MAX_REF_LIST_LENGTH);
    if (index >= _items.size())
        return NULL;
    return _items[index];
}

Term* RefList::operator[](unsigned int index) const
{
    ca_assert(index < MAX_REF_LIST_LENGTH);
    return get(index);
}

Ref& RefList::operator[](unsigned int index)
{
    ca_assert(index < MAX_REF_LIST_LENGTH);
    return _items[index];
}

void RefList::appendAll(RefList const& list)
{
    ca_assert(&list != this);

    for (int i=0; i < list.length(); i++)
        append(list[i]);
}

void RefList::setAt(unsigned int index, Term* term)
{
    ca_assert(index < MAX_REF_LIST_LENGTH);

    // Make sure there are enough blank elements in the list
    while (_items.size() <= index) {
        _items.push_back(NULL);
    }

    _items[index] = term;
}

void RefList::remove(Term* term)
{
    int numRemoved = 0;
    for (size_t i=0; i < _items.size(); i++) {

        if (_items[i] == term) {
            numRemoved++;
        } else if (numRemoved > 0) {
            _items[i - numRemoved] = _items[i];
        }
    }

    _items.resize(_items.size() - numRemoved);
}

void RefList::remove(int index)
{
    _items.erase(_items.begin() + index);
}


void RefList::resize(unsigned int newLength)
{
    ca_assert(newLength < MAX_REF_LIST_LENGTH);
    _items.resize(newLength);
}

void RefList::remapPointers(ReferenceMap const& map)
{
    for (int i=0; i < length(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

} // namespace circa
