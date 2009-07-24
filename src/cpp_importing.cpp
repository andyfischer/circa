// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {
namespace cpp_importing {

void pointer_alloc(Term* type, Term* term)
{
    term->value = NULL;
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
