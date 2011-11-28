// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "for_loop.h"
#include "function.h"
#include "generic.h"
#include "if_block.h"
#include "kernel.h"
#include "subroutine.h"
#include "type_inference.h"

namespace circa {

void create_function_vectorized_vs(Branch* out, Term* func, Type* lhsType, Type* rhsType)
{
    clear_branch(out);

    Term* input0 = apply(out, INPUT_PLACEHOLDER_FUNC, TermList(), "in0");
    change_declared_type(input0, lhsType);
    Term* input1 = apply(out, INPUT_PLACEHOLDER_FUNC, TermList(), "in1");
    change_declared_type(input1, rhsType);

    Term* loop = apply(out, FOR_FUNC, TermList(input0));
    start_building_for_loop(loop, "it");
    Branch* loopContents = nested_contents(loop);

    Term* iterator = loopContents->get("it");
    apply(loopContents, func, TermList(iterator, input1));

    finish_for_loop(loop);

    apply(out, OUTPUT_PLACEHOLDER_FUNC, TermList(loop));
}

void create_function_vectorized_vv(Branch* out, Term* func, Type* lhsType, Type* rhsType)
{
    clear_branch(out);

    Term* input0 = apply(out, INPUT_PLACEHOLDER_FUNC, TermList(), "in0");
    change_declared_type(input0, lhsType);
    Term* input1 = apply(out, INPUT_PLACEHOLDER_FUNC, TermList(), "in1");
    change_declared_type(input1, rhsType);

    Term* loop = apply(out, FOR_FUNC, TermList(input0));
    start_building_for_loop(loop, "it");
    Branch* loopContents = nested_contents(loop);

    Term* iterator = loopContents->get("it");

    Term* index = find_term_with_function(loopContents, BUILTIN_FUNCS.loop_index);
    Term* get_index = apply(loopContents, GET_INDEX_FUNC, TermList(input1, index));
    apply(loopContents, func, TermList(iterator, get_index));

    finish_for_loop(loop);

    apply(out, OUTPUT_PLACEHOLDER_FUNC, TermList(loop));
}

Term* create_overloaded_function(Branch* branch, const char* name, TermList const& functions)
{
    return create_overloaded_function(branch, name, (TermList*) &functions);
}
Term* create_overloaded_function(Branch* branch, const char* name, TermList* functions)
{
    Term* term = create_subroutine(branch, name);
    create_overloaded_function(function_get_contents(as_function(term)), functions);
    return term;
}

void create_overloaded_function(Branch* out, TermList* functions)
{
    ca_assert(functions->length() > 0);

    int inputCount = function_num_inputs(as_function(functions->get(0)));

    for (int i=0; i < functions->length(); i++) {
        ca_assert(inputCount == function_num_inputs(as_function(functions->get(i))));
    }

    TermList inputPlaceholders;

    // Add input placeholders
    for (int inputIndex=0; inputIndex < inputCount; inputIndex++) {

        Type* type = NULL;

        for (int i=0; i < functions->length(); i++) {
            Term* placeholder = function_get_input_placeholder(
                as_function(functions->get(i)), inputIndex);
            type = find_common_type(type, placeholder->type);
        }

        Term* placeholder = apply(out, INPUT_PLACEHOLDER_FUNC, TermList());
        change_declared_type(placeholder, type);

        inputPlaceholders.append(placeholder);
    }

    // Take the (possibly variadic) args and create a proper list.
    Term* inputsAsList = apply(out, LIST_FUNC, inputPlaceholders);

    // Add the switch block
    Term* block = apply(out, IF_BLOCK_FUNC, TermList());
    Branch* blockContents = nested_contents(block);

    for (int i=0; i < functions->length(); i++) {
        Term* function = functions->get(i);
        Term* condition = apply(out, BUILTIN_FUNCS.inputs_fit_function,
            TermList(inputsAsList, function));
        move_before(condition, block);
        Term* caseTerm = apply(blockContents, CASE_FUNC, TermList(condition));
        apply(nested_contents(caseTerm), function, inputPlaceholders);
    }

    finish_if_block(block);
    apply(out, OUTPUT_PLACEHOLDER_FUNC, TermList(block));
}

void append_to_overloaded_function(Branch* func, Term* function)
{
    // TODO
}

bool is_overloaded_function(Function* func)
{
    // TODO
    return false;
}

} // namespace circa
