// Copyright 2008 Andrew Fischer

#ifndef CIRCA_COMPOUND_VALUE_INCLUDED
#define CIRCA_COMPOUND_VALUE_INCLUDED

#include "pointer_iterator.h"

namespace circa {

// This is a dumb thing that allows for some runtime type assertion.
const int COMPOUND_TYPE_SIGNATURE = 0x43214321;

struct CompoundValue
{
    int signature;
    ReferenceList fields;

    // Member functions
    CompoundValue() : signature(COMPOUND_TYPE_SIGNATURE) {}
    ~CompoundValue();

    Term* appendSlot(Term* type);

    // Hosted functions
    static void* alloc(Term* typeTerm);
    static void dealloc(void* data);
    static PointerIterator* start_pointer_iterator(Term* term);

    static void create_compound_type(Term* term);
    static void append_field(Term* term);
};

CompoundValue& as_compound_value(Term *term);

Term* get_field(Term *term, std::string const& fieldName);
Term* get_field(Term *term, int index);

PointerIterator* start_compound_value_pointer_iterator(CompoundValue* value);

} // namespace circa

#endif
