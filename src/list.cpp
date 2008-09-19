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

void
List::append(Term* term)
{
    Term* newTerm = create_constant(&this->branch, term->type);
    recycle_value(term, newTerm);
    this->items.append(newTerm);
}

void
List::clear()
{
    items.clear();
    branch.clear();
}

ReferenceList
List::toReferenceList()
{
    ReferenceList result;

    for (int i=0; i < items.count(); i++)
        result.append(as_ref(items[i]));

    return result;
}

List& as_list(Term* term)
{
    if (term->type != LIST_TYPE)
        throw errors::TypeError(term, LIST_TYPE);
    return *((List*) term->value);
}

void initialize_list(Branch* kernel)
{
    LIST_TYPE = quick_create_cpp_type<List>(kernel, "List");
}

} // namespace circa
