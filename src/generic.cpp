// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "loops.h"
#include "function.h"
#include "generic.h"
#include "if_block.h"
#include "kernel.h"
#include "parser.h"
#include "reflection.h"
#include "type.h"
#include "type_inference.h"

namespace circa {

void create_function_vectorized_vs(Branch* out, Term* func, Type* lhsType, Type* rhsType)
{
    clear_branch(out);

    Term* input0 = apply(out, FUNCS.input, TermList(), "in0");
    change_declared_type(input0, lhsType);
    Term* input1 = apply(out, FUNCS.input, TermList(), "in1");
    change_declared_type(input1, rhsType);

    Term* loop = apply(out, FUNCS.for_func, TermList(input0));
    start_building_for_loop(loop, "it", NULL);
    Branch* loopContents = nested_contents(loop);

    Term* iterator = loopContents->get("it");
    apply(loopContents, func, TermList(iterator, input1));

    finish_for_loop(loop);

    apply(out, FUNCS.output, TermList(loop));
}

void create_function_vectorized_vv(Branch* out, Term* func, Type* lhsType, Type* rhsType)
{
    clear_branch(out);

    Term* input0 = apply(out, FUNCS.input, TermList(), "in0");
    change_declared_type(input0, lhsType);
    Term* input1 = apply(out, FUNCS.input, TermList(), "in1");
    change_declared_type(input1, rhsType);

    Term* loop = apply(out, FUNCS.for_func, TermList(input0));
    start_building_for_loop(loop, "it", NULL);
    Branch* loopContents = nested_contents(loop);

    Term* iterator = loopContents->get("it");

    Term* index = find_term_with_function(loopContents, FUNCS.loop_index);
    Term* get_index = apply(loopContents, FUNCS.get_index, TermList(input1, index));
    apply(loopContents, func, TermList(iterator, get_index));

    finish_for_loop(loop);

    apply(out, FUNCS.output, TermList(loop));
}

Term* create_overloaded_function(Branch* branch, const char* header)
{
    Term* function = parser::compile(branch, parser::function_decl, header);
    Branch* contents = nested_contents(function);

    // Box the inputs in a list (used in calls to inputs_fit_function)
    TermList inputPlaceholders;
    input_placeholders_to_list(contents, &inputPlaceholders);
    Term* inputsAsList = apply(contents, FUNCS.list, inputPlaceholders);

    // Add the switch block
    Term* block = apply(contents, FUNCS.if_block, TermList());

    // Cases are added with append_to_overloaded_function()

    // Setup the fallback case
    Term* elseTerm = if_block_append_case(nested_contents(block), NULL);
    Branch* elseBranch = nested_contents(elseTerm);
    apply(elseBranch, FUNCS.overload_error_no_match, TermList(inputsAsList));
    if_block_finish_appended_case(elseBranch, elseTerm);

    set_input(get_output_placeholder(contents, 0), 0, block);

    function->setBoolProp("overloadedFunc", true);
    function->setBoolProp("preferSpecialize", true);
    return function;
}

void append_to_overloaded_function(Branch* overloadedFunc, Term* specializedFunc)
{
    TermList inputPlaceholders;
    input_placeholders_to_list(overloadedFunc, &inputPlaceholders);

    Term* inputsAsList = find_term_with_function(overloadedFunc, FUNCS.list);
    Term* ifBlock = find_term_with_function(overloadedFunc, FUNCS.if_block);

    Term* condition = apply(overloadedFunc, FUNCS.inputs_fit_function,
        TermList(inputsAsList, specializedFunc));
    move_before(condition, ifBlock);

    Term* caseTerm = if_block_append_case(nested_contents(ifBlock), condition);
    Branch* caseBranch = nested_contents(caseTerm);
    apply(caseBranch, specializedFunc, inputPlaceholders);
    if_block_finish_appended_case(caseBranch, caseTerm);
}

void append_to_overloaded_function(Term* overloadedFunc, Term* specializedFunc)
{
    return append_to_overloaded_function(nested_contents(overloadedFunc), specializedFunc);
}

#if 0
void specialize_overload_for_call(Term* call)
{
    Branch* original = function_contents(call->function);
    Term* switchTerm = find_term_with_function(original, FUNCS.if_block);
    ca_assert(switchTerm != NULL);
    Branch* switchBranch = nested_contents(switchTerm);

    // Find which case will succeed
    Branch* successCase = NULL;
    for (int i=0; i < switchBranch->length(); i++) {
        Term* term = switchBranch->get(i);
        if (term->function != FUNCS.case_func)
            continue;

        // Check if we have reached the fallback case.
        if (term->input(0) == NULL) {
            successCase = NULL;
            break;
        }

        Term* inputCheck = term->input(0);
        Term* func = inputCheck->input(1);

        // Check if this function statically fits
        bool allInputsFit = true;
        for (int inputIndex=0; inputIndex < call->numInputs(); inputIndex++) {
            if (call->input(inputIndex) == NULL)
                continue;

            if (!term_output_always_satisfies_type(call->input(inputIndex),
                function_get_input_type(func, inputIndex))) {
                allInputsFit = false;
                break;
            }
        }

        if (allInputsFit) {
            successCase = nested_contents(term);
            break;
        }
    }

    // If successCase is NULL then no static specialization is possible.
    if (successCase == NULL) {
        remove_nested_contents(call);
        return;
    }

    // Copy the successful case
    clear_branch(nested_contents(call));
    duplicate_branch(successCase, nested_contents(call));

    // Pass along the :multiple property
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(original, i);
        if (placeholder == NULL)
            break;
        Term* localPlaceholder = get_input_placeholder(nested_contents(call), i);

        ca_assert(localPlaceholder != NULL);

        if (placeholder->boolProp("multiple", false))
            localPlaceholder->setBoolProp("multiple", true);
    }

    expand_variadic_inputs_for_call(nested_contents(call), call);
    change_declared_type(call, get_output_placeholder(nested_contents(call), 0)->type);
}
#endif

bool is_overloaded_function(Branch* branch)
{
    if (branch->owningTerm == NULL)
        return false;

    return branch->owningTerm->boolProp("overloadedFunc", false);
}

void list_overload_contents(Branch* branch, caValue* output)
{
    if (!is_overloaded_function(branch))
        return;

    Term* ifBlock = find_term_with_function(branch, FUNCS.if_block);
    Branch* ifContents = nested_contents(ifBlock);

    // iterate across cases
    for (int caseIndex=0;; caseIndex++) {
        Term* caseTerm = if_block_get_case(ifContents, caseIndex);
        if (caseTerm == NULL)
            break;

        Branch* caseContents = nested_contents(caseTerm);

        Term* call = find_last_non_comment_expression(caseContents);
        Term* func = call->function;

        if (func == FUNCS.overload_error_no_match)
            continue;

        set_term_ref(list_append(output), func);
    }
}

} // namespace circa
