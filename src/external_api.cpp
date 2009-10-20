// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void assert_input_index(Term* term, int index)
{
    if (index >= term->numInputs()) {
        throw std::runtime_error("Input index out of range");
    }
}

int int_input(Term* term, int index)
{
    assert_input_index(term, index);
    return as_int(term->input(index));
}

float float_input(Term* term, int index)
{
    assert_input_index(term, index);
    return to_float(term->input(index));
}

bool bool_input(Term* term, int index)
{
    assert_input_index(term, index);
    return as_bool(term->input(index));
}

const char* string_input(Term* term, int index)
{
    assert_input_index(term, index);
    return as_string(term->input(index)).c_str();
}

} // namespace circa
