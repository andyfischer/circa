// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPOUND_VALUE__INCLUDED
#define CIRCA__COMPOUND_VALUE__INCLUDED

#include "branch.h"
#include "ref_list.h"

namespace circa {

struct CompoundValue {
    ReferenceList fields;
    Branch branch;

    Term* addField(Term* type);

    Term* getField(int index) {
        return fields[index];
    }
};

bool is_compound_value(Term*);
CompoundValue& as_compound_value(Term*);

void instantiate_compound_value(CompoundType const &type, CompoundValue &value);

} // namespace circa

#endif
