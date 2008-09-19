// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "errors.h"
#include "term.h"
#include "ref_list.h"
#include "term_map.h"

namespace circa {

void
ReferenceList::appendAll(ReferenceList const& list)
{
    if (&list == this)
        throw errors::InternalError("Circular call");

    for (int i=0; i < list.count(); i++)
        append(list[i]);
}

void ReferenceList::remapPointers(TermMap const& map)
{
    for (int i=0; i < _items.size(); i++)
        _items[i] = map.getRemapped(_items[i]);
}

} // namespace circa
