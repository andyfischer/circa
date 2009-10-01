// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

int int_input(Term* term, int index)
{
    return as_int(term->input(index));
}

float float_input(Term* term, int index)
{
    return to_float(term->input(index));
}

bool bool_input(Term* term, int index)
{
    return as_bool(term->input(index));
}

const char* string_input(Term* term, int index)
{
    return as_string(term->input(index)).c_str();
}

} // namespace circa
