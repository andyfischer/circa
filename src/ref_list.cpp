// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

void RefList::appendAll(RefList const& list)
{
    assert(&list != this);

    for (int i=0; i < list.length(); i++)
        append(list[i]);
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
