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

const char* for_loop_get_iterator_name(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    for (int i=0; i < contents->length(); i++)
        if (contents->get(i)->function == GET_INDEX_FUNC)
            return contents->get(i)->name.c_str();
    return "";
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

Term* start_building_for_loop(Term* forTerm, const char* iteratorName)
{
    Branch* contents = nested_contents(forTerm);

    // Add input placeholder for the list input
    Term* listInput = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList());

    // Add loop_index()
    Term* index = apply(contents, BUILTIN_FUNCS.loop_index, TermList(listInput));
    hide_from_source(index);

    // Add loop_iterator()
    Term* iterator = apply(contents, GET_INDEX_FUNC, TermList(listInput, index),
        iteratorName);
    change_declared_type(iterator, infer_type_of_get_index(forTerm->input(0)));
    hide_from_source(iterator);
    return iterator;
}

void add_loop_output_term(Branch* branch)
{
    Term* result = find_last_non_comment_expression(branch);
    Term* term = apply(branch, BUILTIN_FUNCS.loop_output, TermList(result));
    move_before_outputs(term);
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

        set_input(forTerm, inputIndex, original);

        // Repoint terms to use our new input_placeholder
        for (int i=0; i < contents->length(); i++)
            remap_pointers_quick(contents->get(i), original, input);

        apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(result), name);

        inputIndex++;
    }
}

void repoint_terms_to_use_input_placeholders(Branch* contents)
{
    // Visit every term
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        // Visit every input
        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            Term* input = term->input(inputIndex);
            if (input == NULL)
                continue;
            
            // If the input is outside this branch, then see if we have a named
            // input that could be used instead.
            if (input->owningBranch == contents || input->name == "")
                continue;

            Term* replacement = find_input_placeholder_with_name(contents, input->name.c_str());
            if (replacement == NULL)
                continue;

            set_input(term, inputIndex, replacement);
        }
    }
}

void finish_for_loop(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    add_loop_output_term(contents);

    add_implicit_placeholders(forTerm);
    repoint_terms_to_use_input_placeholders(contents);

    // Add a primary output
    apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL));

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

    TaggedValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    // Preserve old for-loop context
    ForLoopContext prevLoopContext = context->forLoopContext;
    context->forLoopContext.discard = false;

    List registers;
    registers.resize(contents->length());

    // Copy inputs (first time)
    for (int i=0;; i++) {
        if (get_input_placeholder(contents, i) != NULL)
            copy(INPUT(i), registers[i]);
        else
            break;
    }

    // Create a stack frame
    push_frame(context, contents, &registers);

    // Walk forward until we find the loop_index() term.
    int loopIndexPos = 0;
    for (; loopIndexPos < contents->length(); loopIndexPos++) {
        if (contents->get(loopIndexPos)->function == BUILTIN_FUNCS.loop_index)
            break;
    }
    ca_assert(contents->get(loopIndexPos)->function == BUILTIN_FUNCS.loop_index);

    // Find the loop_output() term.
    int loopOutputPos = -1;
    for (int i=0; i < contents->length(); i++) {
        if (contents->get(i)->function == BUILTIN_FUNCS.loop_output) {
            loopOutputPos = contents->get(i)->input(0)->index;
            break;
        }
    }
    ca_assert(loopOutputPos > 0);

    List loopOutput;

    // For a zero-iteration loop, copy over inputs to their respective outputs.
    if (inputListLength == 0) {
        List* registers = &top_frame(context)->registers;
        for (int i=1;; i++) {
            Term* input = get_input_placeholder(contents, i);
            if (input == NULL)
                break;
            Term* output = get_output_placeholder(contents, i);
            ca_assert(output != NULL);
            copy(registers->get(input->index), registers->get(output->index));
        }
    }

    for (int iteration=0; iteration < inputListLength; iteration++) {
        context->forLoopContext.continueCalled = false;

        // Set the loop index
        set_int(top_frame(context)->registers[loopIndexPos], iteration);

        // Evaluate contents, skipping past loopIndexPos
        for (int i=loopIndexPos+1; i < contents->length(); i++) {
            if (evaluation_interrupted(context))
                break;

            evaluate_single_term(context, contents->get(i));
        }

        // Copy loop output
        copy(top_frame(context)->registers[loopOutputPos], loopOutput.append());

        // Check if we are finished
        if (iteration >= inputListLength)
            break;

        // If we're not finished yet, copy rebound outputs back to inputs.
        for (int i=1;; i++) {
            List* registers = &top_frame(context)->registers;
            Term* input = get_input_placeholder(contents, i);
            if (input == NULL)
                break;
            Term* output = get_output_placeholder(contents, i);
            copy(registers->get(output->index), registers->get(input->index));
        }
    }

    // If an error occurred, leave context the way it was.
    if (context->errorOccurred)
        return;

    // Restore loop context
    context->forLoopContext = prevLoopContext;

    swap(&top_frame(context)->registers, &registers);
    pop_frame(context);

    // Save outputs
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(contents, i);
        if (placeholder == NULL)
            break;

        copy(registers[placeholder->index], OUTPUT_NTH(i));
    }

    copy(&loopOutput, OUTPUT);
}

} // namespace circa
