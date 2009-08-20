// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

using namespace circa;

extern "C" {

int int_input(Term* term, int index)
{
    return as_int(term->input(index));
}

float float_input(Term* term, int index)
{
    return as_float(term->input(index));
}

bool bool_input(Term* term, int index)
{
    return as_bool(term->input(index));
}

const char* string_input(Term* term, int index)
{
    return as_string(term->input(index)).c_str();
}

} // extern "C"
