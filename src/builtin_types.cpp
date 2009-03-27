// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"


namespace circa {

int& as_int(Term* term)
{
    assert_type(term, INT_TYPE);
    assert(term->value != NULL);
    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert_type(term, FLOAT_TYPE);
    assert(term->value != NULL);
    return *((float*) term->value);
}

bool& as_bool(Term* term)
{
    assert_type(term, BOOL_TYPE);
    assert(term->value != NULL);
    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert_type(term, STRING_TYPE);
    assert(term->value != NULL);
    return *((std::string*) term->value);
}

// todo: add these for all primitives
bool is_string(Term* term)
{
    return term->type == STRING_TYPE;
}

Ref& as_ref(Term* term)
{
    assert_type(term, REF_TYPE);
    return *((Ref*) term->value);
}

void*& as_void_ptr(Term* term)
{
    assert_type(term, VOID_PTR_TYPE);
    return term->value;
}

} // namespace circa
