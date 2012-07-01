// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "loops.h"
#include "function.h"
#include "generic.h"
#include "if_block.h"
#include "inspection.h"
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

Term* statically_specialize_overload_for_call(Term* call)
{
    Branch* original = function_contents(call->function);
    Term* switchTerm = find_term_with_function(original, FUNCS.if_block);
    ca_assert(switchTerm != NULL);
    Branch* switchBranch = nested_contents(switchTerm);

    // Do not try to specialize if any arguments are untyped.
    for (int i=0; i < call->numInputs(); i++)
        if (call->input(i) != NULL && declared_type(call->input(i)) == &ANY_T)
            return NULL;

    // Find which case will succeed
    for (int i=0; i < switchBranch->length(); i++) {
        Term* caseTerm = switchBranch->get(i);
        if (caseTerm->function != FUNCS.case_func)
            continue;

        // Stop if we have reached the fallback case.
        if (caseTerm->input(0) == NULL)
            break;

        Term* caseCondition = caseTerm->input(0);
        Term* func = caseCondition->input(1);

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

        if (allInputsFit)
            return func;
    }

    return NULL;
}

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
