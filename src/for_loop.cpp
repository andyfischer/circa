// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "evaluation.h"
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

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
   [1] #inner_rebinds
   [2] iterator
   [...] contents
   [n-1] #outer_rebinds
*/

static const int inner_rebinds_location = 1;
static const int iterator_location = 2;
static const int loop_contents_location = 3;

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->contents(iterator_location);
}

Term* get_for_loop_modify_list(Term* forTerm)
{
    Term* term = forTerm->contents(0)->contents(0);
    ca_assert(term != NULL);
    return term;
}

Branch* get_for_loop_inner_rebinds(Term* forTerm)
{
    return nested_contents(forTerm->contents(1));
}

Branch* get_for_loop_outer_rebinds(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch* forContents = nested_contents(forTerm);
    Branch* attributes = create_branch(forContents, "#attributes");
    create_bool(attributes, false, "#modify_list");

    Branch* innerRebinds = create_branch(forContents, "#inner_rebinds");
    innerRebinds->owningTerm->setBoolProp("exposesNames", true);
}

Term* setup_for_loop_iterator(Term* forTerm, const char* name)
{
    Type* iteratorType = infer_type_of_get_index(forTerm->input(0));
    Term* result = apply(nested_contents(forTerm), INPUT_PLACEHOLDER_FUNC, TermList(), name);
    change_declared_type(result, iteratorType);
    hide_from_source(result);
    return result;
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch* forContents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    std::string iteratorName = get_for_loop_iterator(forTerm)->name;

    finish_minor_branch(forContents);

    // Create a branch that has all the names which are rebound in this loop
    Branch* innerRebinds = get_for_loop_inner_rebinds(forTerm);
    Branch* outerRebinds = create_branch(forContents, "#outer_rebinds");

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(forContents, reboundNames);

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

        Term* loopResult = forContents->get(name);

        // First input to both of these should be 'original', but we need to wait until
        // after remap_pointers before setting this.
        Term* innerRebind = apply(innerRebinds, JOIN_FUNC, TermList(NULL, loopResult), name);
        change_declared_type(innerRebind, original->type);
        Term* outerRebind = apply(outerRebinds, JOIN_FUNC, TermList(NULL, loopResult), name);

        // Rewrite the loop code to use our local copies of these rebound variables.
        remap_pointers(forContents, original, innerRebind);

        set_input(innerRebind, 0, original);
        set_input(outerRebind, 0, original);

        respecialize_type(outerRebind);
    }

    for_loop_update_output_index(forTerm);
    update_input_instructions(forTerm);
    recursively_finish_update_cascade(forContents);
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
    if (as_bool(get_for_loop_modify_list(forTerm))) {
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
    Branch* forContents = nested_contents(caller);
    Branch* innerRebinds = get_for_loop_inner_rebinds(caller);
    Branch* outerRebinds = get_for_loop_outer_rebinds(caller);
    Term* iterator = get_for_loop_iterator(caller);

    TaggedValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    TaggedValue outputTv;
    bool saveOutput = forContents->outputIndex != -1;
    List* output = set_list(&outputTv, inputListLength);
    int nextOutputIndex = 0;

    start_using(forContents);
    start_using(innerRebinds);
    context->callStack.append(CALLER);

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

    for (int iteration=0; iteration < inputListLength; iteration++) {
        context->forLoopContext.continueCalled = false;

        bool firstIter = iteration == 0;

        // load state for this iteration
        if (useState)
            swap(state->get(iteration), &context->currentScopeState);

        // copy iterator
        copy(inputList->getIndex(iteration), get_local(iterator));

        // copy inner rebinds
        for (int i=0; i < innerRebinds->length(); i++) {
            Term* rebindTerm = innerRebinds->get(i);
            TaggedValue* dest = get_local(rebindTerm);

            if (firstIter)
                copy(get_input(context, rebindTerm, 0), dest);
            else
                copy(get_input(context, rebindTerm, 1), dest);
        }

        context->forLoopContext.discard = false;

        for (int i=loop_contents_location; i < forContents->length() - 1; i++) {
            if (evaluation_interrupted(context))
                break;

            evaluate_single_term(context, forContents->get(i));
        }

        // Save output
        if (saveOutput && !context->forLoopContext.discard) {
            TaggedValue* localResult = get_local(forContents->get(forContents->outputIndex));
            copy(localResult, output->get(nextOutputIndex++));
        }

        // Unload state
        if (useState)
            swap(&context->currentScopeState, state->get(iteration));
    }

    // Resize output, in case some elements were discarded
    output->resize(nextOutputIndex);

    swap(output, OUTPUT);

    // Copy outer rebinds
    ca_assert(caller->numOutputs() == outerRebinds->length() + 1);
    
    for (int i=0; i < outerRebinds->length(); i++) {

        Term* rebindTerm = outerRebinds->get(i);

        TaggedValue* result = NULL;

        if (inputListLength == 0) {
            // No iterations, use the outer rebind
            result = get_input(context, rebindTerm, 0);
        } else {
            // At least one iteration, use our local rebind
            result = get_input(context, rebindTerm, 1);
        }

        copy(result, EXTRA_OUTPUT(i));
    }

    // Restore loop context
    context->forLoopContext = prevLoopContext;

    if (useState) {
        save_and_consume_state(CALLER, &prevScopeState, &localState);
        swap(&prevScopeState, &context->currentScopeState);
    }

    context->callStack.pop();
    finish_using(forContents);
    finish_using(innerRebinds);
}

} // namespace circa
