// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "compound_type.h"
#include "compound_value.h"
#include "operations.h"

namespace circa {

void instantiate_compound_value(CompoundType const &type, CompoundValue &value)
{
    value.fields.clear();
    value.branch.clear();

    for (int fieldIndex=0; fieldIndex < type.numFields(); fieldIndex++) {
        Term* term = create_constant(&value.branch, type.getType(fieldIndex));
        value.fields.append(term);
    }
}

}
