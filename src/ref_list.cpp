// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

void RefList::appendAll(RefList const& list)
{
    assert(&list != this);

    for (unsigned int i=0; i < list.count(); i++)
        append(list[i]);
}

void RefList::remapPointers(ReferenceMap const& map)
{
    for (unsigned int i=0; i < _items.size(); i++)
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
