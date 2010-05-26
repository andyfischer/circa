// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtin_types.h"
#include "builtin_types/list.h"
#include "building.h"
#include "term.h"
#include "type.h"

namespace circa {

namespace compound_t {

/*
bool is_subtype(Type* type, Type* otherType)
{
    if (!list_t::is_list_based_type(otherType))
        return false;

    int numElements = type->prototype.length();
    if (numElements != otherType->prototype.length())
        return false;

    Branch& leftPrototype = type->prototype;
    Branch& rightPrototype = otherType->prototype;

    for (int i=0; i < numElements; i++) {
        if (!circa::is_subtype(type_contents(leftPrototype[i]->type),
                type_contents(rightPrototype[i]->type)))
            return false;
    }
    return true;
}*/

}

void initialize_compound_type(Type* type)
{
    list_t::setup_type(type);
  //  type->isSubtype = compound_t::is_subtype;
}

void initialize_compound_type(Term* term)
{
    initialize_compound_type(type_contents(term));
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
#ifdef NEWLIST
    initialize_compound_type(term);
#else
    initialize_branch_based_type(term);
#endif
    type_contents(term)->name = name;
    return term;
}

}
