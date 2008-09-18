// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPOUND_VALUE__INCLUDED
#define CIRCA__COMPOUND_VALUE__INCLUDED

#include "branch.h"
#include "term_list.h"

namespace circa {

struct CompoundValue {
    TermList fields;
    Branch branch;

    Term* append(Term* type);

    Term* getField(int index) {
        return fields[index];
    }
};

bool is_compound_value(Term*);
CompoundValue& as_compound_value(Term*);

void instantiate_compound_value(CompoundType const &type, CompoundValue &value);

} // namespace circa

#endif
