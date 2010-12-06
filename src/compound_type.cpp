// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "building.h"
#include "term.h"
#include "type.h"
#include "types/list.h"

namespace circa {

void initialize_compound_type(Type* type)
{
    list_t::setup_type(type);
}

void initialize_compound_type(Term* term)
{
    initialize_compound_type(type_contents(term));
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
    initialize_compound_type(term);
    type_contents(term)->name = name;
    return term;
}

Type* get_compound_list_element_type(Type* compoundType, int index)
{
    return type_contents(compoundType->prototype[index]->type);
}

}
