// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "introspection.h"
#include "locals.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "refactoring.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "for_loop.h"

namespace circa {

Term* get_for_loop_iterator(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function != INPUT_PLACEHOLDER_FUNC)
            return NULL;
        if (function_is_state_input(term))
            continue;

        return term;
    }
    return NULL;
}

bool for_loop_modifies_list(Term* forTerm)
{
    return forTerm->boolPropOptional("modifyList", false);
}

Branch* get_for_loop_outer_rebinds(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

Term* setup_for_loop_iterator(Term* forTerm, const char* name)
{
    Type* iteratorType = infer_type_of_get_index(forTerm->input(0));
    Term* result = apply(nested_contents(forTerm), INPUT_PLACEHOLDER_FUNC, TermList(), name);
    change_declared_type(result, iteratorType);
    hide_from_source(result);
    return result;
}

void add_implicit_placeholders(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    Term* iterator = get_for_loop_iterator(forTerm);
    std::string iteratorName = iterator->name;

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(contents, reboundNames);

    int inputIndex = 1;

    for (size_t i=0; i < reboundNames.size(); i++) {
        std::string const& name = reboundNames[i];
        if (name == listName)
            continue;
        if (name == iteratorName)
            continue;

        Term* original = get_named_at(forTerm, name);

        // The name might not be found, for certain parser errors.
        if (original == NULL)
            continue;

        Term* result = contents->get(name);

        Term* input = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList(), name);
        change_declared_type(input, original->type);
        contents->move(input, inputIndex);

        // Repoint terms to use our new input_placeholder
        for (int i=0; i < contents->length(); i++)
            remap_pointers_quick(contents->get(i), original, input);

        apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(result), name);
    }
}

void finish_for_loop(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    add_implicit_placeholders(forTerm);

    // Add a primary output
    apply(contents, OUTPUT_PLACEHOLDER_FUNC,
        TermList(find_last_non_comment_expression(contents)));

    check_to_insert_implicit_inputs(forTerm);
}

Term* find_enclosing_for_loop(Term* term)
{
    if (term == NULL)
        return NULL;

    if (term->function == FOR_FUNC)
        return term;

    Branch* branch = term->owningBranch;
    if (branch == NULL)
        return NULL;

    return find_enclosing_for_loop(branch->owningTerm);
}

void for_loop_update_output_index(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    // If this is a list-rewrite, then the output is the last term that has the iterator's
    // name binding. Otherwise the output is the last expression.
    if (for_loop_modifies_list(forTerm)) {
        Term* output = contents->get(get_for_loop_iterator(forTerm)->name);
        ca_assert(output != NULL);
        contents->outputIndex = output->index;
    } else {
        // Find the first non-comment expression before #outer_rebinds
        Term* output = find_last_non_comment_expression(contents);
        contents->outputIndex = output == NULL ? -1 : output->index;
    }
}

CA_FUNCTION(evaluate_for_loop)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    Term* iterator = get_for_loop_iterator(caller);

    TaggedValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    TaggedValue outputTv;
    List* output = set_list(&outputTv, inputListLength);
    int nextOutputIndex = 0;

    Frame *frame = push_frame(context, contents);

    // Prepare state container
    bool useState = has_implicit_state(CALLER);
    TaggedValue localState;
    TaggedValue prevScopeState;
    List* state = NULL;
    if (useState) {
        swap(&context->currentScopeState, &prevScopeState);
        fetch_state_container(CALLER, &prevScopeState, &localState);

        state = List::lazyCast(&localState);
        state->resize(inputListLength);
    }

    // Preserve old for-loop context
    ForLoopContext prevLoopContext = context->forLoopContext;
    context->forLoopContext.discard = false;

    for (int iteration=0; iteration < inputListLength; iteration++) {
        context->forLoopContext.continueCalled = false;

        // copy iterator
        copy(inputList->getIndex(iteration), frame->registers[iterator->index]);

        for (int i=0; i < contents->length(); i++) {
            if (evaluation_interrupted(context))
                break;

            evaluate_single_term(context, contents->get(i));
        }

#if 0
        frame = top_frame(context);

        // Save output
        if (saveOutput && !context->forLoopContext.discard) {
            Term* localResult = contents->get(contents->outputIndex);
            copy(frame->registers[localResult->index], output->get(nextOutputIndex++));
        }

        // Unload state
        if (useState)
            swap(&context->currentScopeState, state->get(iteration));
#endif
    }

    // Resize output, in case some elements were discarded
    output->resize(nextOutputIndex);

    // Copy outer rebinds
    for (int i=0; i < outerRebinds->length(); i++) {

        Term* rebindTerm = outerRebinds->get(i);

        Term* rebindInput = NULL;

        if (inputListLength == 0) {
            // No iterations, use the outer rebind
            rebindInput = rebindTerm->input(0);

        } else {
            // At least one iteration, use our local rebind
            rebindInput = rebindTerm->input(1);
        }

        // Currently this is fairly awkward..
        TaggedValue inputIsn;
        write_input_instruction(rebindTerm, rebindInput, &inputIsn);

        Term* outputTerm = caller->owningBranch->get(caller->index + 1 + i);
        Frame* upperFrame = get_frame(CONTEXT, 1);

        copy(get_arg(CONTEXT, &inputIsn), upperFrame->registers[outputTerm->index]);
    }

    // Restore loop context
    context->forLoopContext = prevLoopContext;

    if (useState) {
        save_and_consume_state(caller, &prevScopeState, &localState);
        swap(&prevScopeState, &context->currentScopeState);
    }

    pop_frame(context);
    swap(output, OUTPUT);
}

} // namespace circa
