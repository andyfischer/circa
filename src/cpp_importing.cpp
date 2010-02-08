// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cpp_importing {

bool raw_value_less_than(Term* a, Term* b)
{
    return a->value_data.asint < b->value_data.asint;
}

} // namespace cpp_importing
} // namespace circa
