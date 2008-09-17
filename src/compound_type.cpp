// Copyright 2008 Andrew Fischer

#include "builtins.h"
#include "compound_type.h"
#include "cpp_interface.h"
#include "errors.h"
#include "term.h"
#include "term_map.h"

namespace circa {

std::string CompoundType__ToString(Term* term);

std::string CompoundType__ToString(Term* term)
{
    // todo
    return "(CompoundType)";
}

bool is_compound_type(Term* term)
{
    return term->type == COMPOUND_TYPE_TYPE;
}

CompoundType& as_compound_type(Term* term)
{
    if (!is_compound_type(term))
        throw errors::TypeError(term, COMPOUND_TYPE_TYPE);
    
    return *((CompoundType*) term->value);
}

void initialize_compound_type(Branch* kernel)
{
    COMPOUND_TYPE_TYPE = quick_create_type(KERNEL, "CompoundType",
            cpp_interface::templated_alloc<CompoundType>,
            cpp_interface::templated_dealloc<CompoundType>,
            cpp_interface::templated_duplicate<CompoundType>,
            CompoundType__ToString);
    as_type(COMPOUND_TYPE_TYPE)->parentType = TYPE_TYPE;
}

} // namespace circa
