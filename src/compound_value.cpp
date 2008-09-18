// Copyright 2008 Paul Hodge

#include "bootstrapping.h"
#include "builtins.h"
#include "common_headers.h"
#include "compound_type.h"
#include "compound_value.h"
#include "cpp_interface.h"
#include "operations.h"
#include "parser.h"

namespace circa {

Term*
CompoundValue::append(Term* type)
{
    Term* field  = create_constant(&this->branch, type);
    this->fields.append(field);
    return field;
}

bool is_compound_value(Term* term)
{
    return term->type == COMPOUND_VALUE_TYPE;
}

CompoundValue& as_compound_value(Term* term)
{
    if (!is_compound_value(term))
        throw errors::TypeError(term, LIST_TYPE);
    return *((CompoundValue*) term->value);
}

// Bootstrap

void instantiate_compound_value(CompoundType const &type, CompoundValue &value)
{
    value.fields.clear();
    value.branch.clear();

    for (int fieldIndex=0; fieldIndex < type.numFields(); fieldIndex++) {
        Term* term = create_constant(&value.branch, type.getType(fieldIndex));
        value.fields.append(term);
    }
}

void CompoundValue__create__evaluate(Term* caller)
{
    TermList &types = as_list(caller->inputs[0]);
    CompoundValue &cvalue = as_compound_value(caller);

    for (int i=0; i < types.count(); i++) {
        Term* term = create_constant(&cvalue.branch, types[i]);
    }
}

Term* quickly_make_compound_value(Branch* branch, TermList types)
{
    Term *term = create_constant(branch, COMPOUND_VALUE_TYPE);
    CompoundValue &value = as_compound_value(term);

    for (int i=0; i < types.count(); i++)
        value.append(types[i]);

    return term;
}

Term* bootstrapped_make_compound_type(Branch* branch)
{
    Term* compoundType = create_constant(branch, COMPOUND_VALUE_TYPE);
    as_compound_value(compoundType).append(REFERENCE_TYPE);
    Term* fieldList = as_compound_value(compoundType).append(LIST_TYPE);

    // todo

    return compoundType;
}

void initialize_compound_value(Branch& kernel)
{
    // Bootstrap
    COMPOUND_VALUE_TYPE = quick_create_cpp_type<CompoundValue>(&kernel, "CompoundType");

    quick_create_function(&kernel, "compound-value",
            CompoundValue__create__evaluate,
            TermList(LIST_TYPE), COMPOUND_VALUE_TYPE);

    // Create CompoundType
    Term* CompoundType = eval_statement(kernel, "CompoundType = CompoundValue()");
    CompoundType = eval_statement(kernel, "compound-value-create-field(@CompoundType, Ref)");
    CompoundType = eval_statement(kernel, "compound-value-create-field(@CompoundType, List)");
}

} // namespace circa
