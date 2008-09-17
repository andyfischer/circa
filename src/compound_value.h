// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPOUND_VALUE__INCLUDED
#define CIRCA__COMPOUND_VALUE__INCLUDED

#include "branch.h"
#include "common_headers.h"

namespace circa {

struct CompoundValue {
    Branch branch;

    Term* getField(int index) {
        return branch[index];
    }
};

bool is_compound_value(Term*);
CompoundValue& as_compound_value(Term*);

} // namespace circa

#endif
