// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "closures.h"
#include "function.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "native_patch.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "type_inference.h"
#include "world.h"

namespace circa {

Term* create_function(Block* block, const char* name)
{
    ca_assert(name != NULL);
    Term* term = apply(block, FUNCS.function_decl, TermList(), name);
    return term;
}

Type* find_implicit_output_type(Block* block)
{
    Type* type = declared_type(get_output_placeholder(block, 0)->input(0));

    // The implicit type must also consider any return statements.
    for (MinorBlockIterator it(block); it; ++it) {
        Term* term = *it;
        if (term->function == FUNCS.return_func)
            type = find_common_type(type, declared_type(term->input(0)));
    }
    return type;
}

void update_static_closure_force(Term* term)
{
    ca_assert(TYPES.func != NULL);

    Block* func = term->nestedContents;

    if (count_closure_upvalues(func) != 0) {
        printf("function has upvalues: %s\n", func->name());
        internal_error("Can't assign a static closure, function has upvalues");
    }

    set_closure(term_value(term), func, NULL);
}

void update_static_closure_if_possible(Term* term)
{
    if (TYPES.func == NULL)
        return;

    Block* func = term->nestedContents;

    if (count_closure_upvalues(func) != 0)
        return;

    set_closure(term_value(term), func, NULL);
}

Value* function_find_output_name_from_inputs(Block* contents)
{
    for (int i = 0;; i++) {
        Term* input = get_input_placeholder(contents, i);
        if (input == NULL)
            break;

        if (input->boolProp(s_Output, false))
            return term_name(input);
    }
    return NULL;
}

void finish_building_function(Block* contents)
{
    // Connect the primary output placeholder. If an input has @ syntax then use the
    // last term with that name. Otherwise, use the last expression.
    Term* primaryOutput = get_output_placeholder(contents, 0);
    if (primaryOutput->input(0) == NULL) {

        Term* result = NULL;

        Value* outputName = function_find_output_name_from_inputs(contents);
        if (outputName != NULL) {
            result = find_name(contents, outputName);
        } else {
            result = find_expression_for_implicit_output(contents);
        }


        if (result != NULL) {
            set_input(primaryOutput, 0, result);
            if (!primaryOutput->boolProp(s_ExplicitType, false))
                set_declared_type(primaryOutput, find_implicit_output_type(contents));
        }
    }

    update_for_control_flow(contents);
    insert_upvalue_terms(contents);
    if (contents->owningTerm != NULL)
        update_static_closure_if_possible(contents->owningTerm);
    block_finish_changes(contents);
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return TYPES.any;

    Block* contents = nested_contents(function);
    Type* outputType = get_output_type(contents, 0);

    if (contents->overrides.specializeType != NULL)
        outputType = contents->overrides.specializeType(call);
    if (outputType == NULL)
        outputType = TYPES.any;

    return outputType;
}

} // namespace circa
