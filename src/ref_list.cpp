// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

const size_t MAX_REF_LIST_LENGTH = 10000000;

void RefList::append(Term* term)
{
    assert(_items.size() < MAX_REF_LIST_LENGTH);
    _items.push_back(term);
}

void RefList::prepend(Term* term)
{
    assert(_items.size() < MAX_REF_LIST_LENGTH);
    _items.insert(_items.begin(), term);
}

void RefList::insert(int index, Term* term)
{
    _items.insert(_items.begin()+index, term);
}

void RefList::appendUnique(Term* term)
{
    for (int i=0; i < length(); i++)
        if (get(i) == term)
            return;

    append(term);
}

int RefList::length() const
{
    assert(_items.size() < MAX_REF_LIST_LENGTH);
    return (int) _items.size();
}

Term* RefList::get(unsigned int index) const
{
    assert(index < MAX_REF_LIST_LENGTH);
    if (index >= _items.size())
        return NULL;
    return _items[index];
}

Term* RefList::operator[](unsigned int index) const
{
    assert(index < MAX_REF_LIST_LENGTH);
    return get(index);
}

Ref& RefList::operator[](unsigned int index)
{
    assert(index < MAX_REF_LIST_LENGTH);
    return _items[index];
}

void RefList::appendAll(RefList const& list)
{
    assert(&list != this);

    for (int i=0; i < list.length(); i++)
        append(list[i]);
}

void RefList::setAt(unsigned int index, Term* term)
{
    assert(index < MAX_REF_LIST_LENGTH);

    // Make sure there are enough blank elements in the list
    while (_items.size() <= index) {
        _items.push_back(NULL);
    }

    _items[index] = term;
}

void RefList::remove(Term* term)
{
    std::vector<Ref>::iterator it;

    for (it = _items.begin(); it != _items.end(); ) {

        if (*it == term)
            it = _items.erase(it);
        else
            ++it;
    }
}

void RefList::remove(int index)
{
    _items.erase(_items.begin() + index);
}


void RefList::resize(unsigned int newLength)
{
    assert(newLength < MAX_REF_LIST_LENGTH);
    _items.resize(newLength);
}

void RefList::remapPointers(ReferenceMap const& map)
{
    for (int i=0; i < length(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

bool compare_by_name(Ref left, Ref right)
{
    return left->name < right->name;
}

void sort_by_name(RefList& list)
{
    std::sort(list._items.begin(), list._items.end(), compare_by_name);
}

} // namespace circa
