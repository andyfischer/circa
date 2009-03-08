// Copyright 2008 Paul Hodge

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

#include "branch.h"
#include "ref_list.h"
#include "term.h"

namespace circa {

namespace set_t {
    void add(Branch& branch, Term* value);
}

void initialize_builtin_types(Branch& kernel);

} // namespace circa

#endif
