// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "building.h"
#include "term.h"
#include "type.h"
#include "types/list.h"

namespace circa {

void initialize_compound_type(Type* type)
{
    list_t::setup_type(type);
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
    initialize_compound_type(unbox_type(term));
    unbox_type(term)->name = name;
    return term;
}

Type* get_compound_list_element_type(Type* compoundType, int index)
{
    return unbox_type(compoundType->prototype[index]->type);
}

}
