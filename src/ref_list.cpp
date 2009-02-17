// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "importing.h"
#include "term.h"
#include "ref_list.h"
#include "ref_map.h"

namespace circa {

void
RefList::appendAll(RefList const& list)
{
    if (&list == this)
        throw std::runtime_error("Circular call");

    for (unsigned int i=0; i < list.count(); i++)
        append(list[i]);
}

void RefList::remapPointers(ReferenceMap const& map)
{
    for (unsigned int i=0; i < _items.size(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

void RefList::visitPointers(PointerVisitor &visitor)
{
    for (unsigned int i=0; i < _items.size(); i++)
        visitor.visitPointer(_items[i]);
}

} // namespace circa
