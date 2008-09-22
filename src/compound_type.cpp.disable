// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "compound_type.h"
#include "cpp_interface.h"
#include "errors.h"
#include "operations.h"
#include "term.h"
#include "term_map.h"
#include "values.h"

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

void CompoundType__add_field__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_type(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);

    as_compound_type(caller).addField(caller->inputs[1], name);
}

void initialize_compound_type(Branch* kernel)
{
    COMPOUND_TYPE_TYPE = quick_create_type(kernel, "CompoundType",
            cpp_interface::templated_alloc<CompoundType>,
            cpp_interface::templated_dealloc<CompoundType>,
            cpp_interface::templated_duplicate<CompoundType>,
            CompoundType__ToString);
    as_type(COMPOUND_TYPE_TYPE)->parentType = TYPE_TYPE;

    quick_create_function(kernel, "compound-type-add-field",
            CompoundType__add_field__evaluate,
            ReferenceList(COMPOUND_TYPE_TYPE, TYPE_TYPE, STRING_TYPE),
            COMPOUND_TYPE_TYPE);
}


} // namespace circa
