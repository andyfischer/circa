// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {
namespace cpp_importing {

void* pointer_alloc(Term* typeTerm)
{
    return NULL;
}

void pointer_dealloc(void* data)
{
}

void raw_value_assign(Term* a, Term* b)
{
    b->value = a->value;
}

bool raw_value_equals(Term* a, Term* b)
{
    return a->value == b->value;
}

bool raw_value_less_than(Term* a, Term* b)
{
    return a->value < b->value;
}

} // namespace cpp_importing
} // namespace circa
