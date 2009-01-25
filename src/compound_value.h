// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPOUND_VALUE_INCLUDED
#define CIRCA_COMPOUND_VALUE_INCLUDED

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

CompoundValue& as_compound_value(Term *term);

Term* get_field(Term *term, std::string const& fieldName);
Term* get_field(Term *term, int index);

} // namespace circa

#endif
