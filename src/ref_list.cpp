// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "errors.h"
#include "term.h"
#include "ref_list.h"
#include "ref_map.h"

namespace circa {

void
ReferenceList::appendAll(ReferenceList const& list)
{
    if (&list == this)
        throw std::runtime_error("Circular call");

    for (unsigned int i=0; i < list.count(); i++)
        append(list[i]);
}

void ReferenceList::remapPointers(ReferenceMap const& map)
{
    for (unsigned int i=0; i < _items.size(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

} // namespace circa
