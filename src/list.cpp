// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "errors.h"
#include "list.h"
#include "operations.h"
#include "values.h"

namespace circa {

List::List(List const& copy)
{
    for (int i=0; i < copy.count(); i++) {
        Term* term = appendSlot(copy[i]->type);
        duplicate_value(copy[i], term);
    }
}

Term*
List::append(Term* term)
{
    Term* newTerm = create_constant(&this->branch, term->type);
    recycle_value(term, newTerm);
    this->items.append(newTerm);
    return newTerm;
}

Term*
List::appendSlot(Term* type)
{
    Term* newTerm = create_constant(&this->branch, type);
    this->items.append(newTerm);
    return newTerm;
}

bool is_list(Term* term)
{
    return (term->type == LIST_TYPE || term->type == COMPOUND_TYPE);
}

List& as_list(Term* term)
{
    if (!is_list(term))
        throw errors::TypeError(term, LIST_TYPE);
    return *((List*) term->value);
}

} // namespace circa
