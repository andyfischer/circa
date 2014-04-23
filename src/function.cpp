// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "closures.h"
#include "function.h"
#include "generic.h"
#include "kernel.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "native_patch.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"
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

void finish_building_function(Block* contents)
{
    // Connect the primary output placeholder with the last expression.
    Term* primaryOutput = get_output_placeholder(contents, 0);
    ca_assert(primaryOutput->input(0) == NULL);
    Term* lastExpression = find_expression_for_implicit_output(contents);
    if (lastExpression != NULL) {
        set_input(primaryOutput, 0, lastExpression);
        if (!primaryOutput->boolProp(sym_ExplicitType, false))
            set_declared_type(primaryOutput, find_implicit_output_type(contents));
    }

    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = count_input_placeholders(contents) - 1; i >= 0; i--) {
        Term* input = get_input_placeholder(contents, i);

        if (input->boolProp(sym_Output, false)) {

            Term* result = find_name(contents, input->name.c_str());
            
            Term* output = append_output_placeholder(contents, result);
            rename(output, &input->nameValue);
            set_declared_type(output, input->type);
            output->setIntProp(sym_RebindsInput, i);
        }
    }

    // After the output_placeholder terms are created, we might need to update any
    // recursive calls.

    for (BlockIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (function_contents(term->function) != contents)
            continue;

        // Update extra outputs
        update_extra_outputs(term);
    }

    update_for_control_flow(contents);
    insert_nonlocal_terms(contents);

    // Possibly apply a native patch
    native_patch_apply_to_new_function(global_world(), contents);

    block_finish_changes(contents);
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return TYPES.any;

    Block* contents = function_contents(function);
    Type* outputType = get_output_type(contents, 0);

    if (contents->overrides.specializeType != NULL)
        outputType = contents->overrides.specializeType(call);
    if (outputType == NULL)
        outputType = TYPES.any;

    if (function->boolProp(sym_PreferSpecialize, false)) {
        Term* specialized = statically_specialize_overload_for_call(call);
        if (specialized != NULL)
            return get_output_type(function_contents(specialized), 0);
    }
    return outputType;
}

void evaluate_subroutine(caStack*)
{
    // This once did something, but now the default function calling behavior
    // is the same as evaluating a subroutine.
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return function_contents(term)->overrides.evaluate == NULL;
}

bool is_subroutine(Block* block)
{
    return block->owningTerm != NULL && is_subroutine(block->owningTerm);
}

} // namespace circa
