// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"
#include "types/list.h"

namespace circa {

struct StaticErrorCheck
{
    List errors;

    // Structure of each item in errors:
    //  [0] int index
    //  [1] string type

    int count() { return errors.length(); }
};

void check_for_static_errors(StaticErrorCheck* result, Branch& branch);

} // namespace circ
