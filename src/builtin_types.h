// Copyright 2008 Paul Hodge

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

#include "branch.h"
#include "ref_list.h"
#include "term.h"

namespace circa {

const int COMPOUND_TYPE_SIGNATURE = 0x43214321;

struct CompoundValue
{
    int signature;
    Branch branch;
    ReferenceList fields;

    // Member functions
    CompoundValue() : signature(COMPOUND_TYPE_SIGNATURE) {}

    Term* appendSlot(Term* type);

    // Static functions
    static void* alloc(Term* typeTerm);
    static void dealloc(void* data);
    static void create_compound_type(Term* term);
    static void append_field(Term* term);
};

void initialize_builtin_types(Branch& kernel);

Term* get_field(Term *term, std::string const& fieldName);
Term* get_field(Term *term, int index);
CompoundValue& as_compound_value(Term *term);

} // namespace circa

#endif
