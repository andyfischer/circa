// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "term.h"

#include "locals.h"

namespace circa {

int get_output_count(Term* term)
{
    if (!FINISHED_BOOTSTRAP)
        return 1;

    // check if the function has overridden getOutputCount
    FunctionAttrs::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;

    FunctionAttrs* attrs = get_function_attrs(term->function);

    if (attrs == NULL)
        return 1;
    
    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term);

    // Default behavior, if FunctionAttrs was found.
    return attrs->outputCount;
}
    
void update_locals_index_for_new_term(Term* term)
{
    Branch* branch = term->owningBranch;

    term->outputCount = get_output_count(term);

    // make sure localsIndex is -1 so that if get_locals_count looks at this
    // term, it doesn't get confused.
    term->localsIndex = -1;
    if (term->outputCount > 0)
        term->localsIndex = get_locals_count(*branch);
}

int get_locals_count(Branch& branch)
{
    if (branch.length() == 0)
        return 0;

    int lastLocal = branch.length() - 1;

    while (branch[lastLocal] == NULL || branch[lastLocal]->localsIndex == -1) {
        lastLocal--;
        if (lastLocal < 0)
            return 0;
    }

    Term* last = branch[lastLocal];

    return last->localsIndex + get_output_count(last);
}

void refresh_locals_indices(Branch& branch, int startingAt)
{
    int nextLocal = 0;
    if (startingAt > 0) {
        Term* prev = branch[startingAt - 1];
        nextLocal = prev->localsIndex + get_output_count(prev);
    }

    for (int i=startingAt; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term == NULL)
            continue;
        term->localsIndex = nextLocal;

        int newOutputCount = get_output_count(term);
        if (term->outputCount != newOutputCount) {
            term->outputCount = newOutputCount;
            update_input_instructions(term);
        }

        term->outputCount = get_output_count(term);
        nextLocal += term->outputCount;
    }
}

void update_output_count(Term* term)
{
    term->outputCount = get_output_count(term);
}

int get_frame_distance(Term* term, Term* input)
{
    if (input == NULL)
        return -1;

    // TODO: Walk 'input' upwards as long as it's in a function that shares registers.

    // Walk upward from 'term' until we find the common branch.
    int distance = 0;
    while (term->owningBranch != input->owningBranch) {
        term = get_parent_term(term);

        if (term == NULL)
            return -1;
    }
    return distance;
}

void update_input_instructions(Term* term)
{
    InputInstructionList& list = term->inputIsns;

    list.inputs.resize(term->numInputs());
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input == NULL) {
            list.inputs[i].type = InputInstruction::EMPTY;
            continue;
        }

        if (is_value(input)) {
            list.inputs[i].type = InputInstruction::GLOBAL;
            continue;
        }

        // TEMP
        list.inputs[i].type = InputInstruction::OLD_STYLE_LOCAL;
        #if 0
        list.inputs[i].relativeFrame =  get_frame_distance(term, input);
        list.inputs[i].index = input->localsIndex + term->inputInfo(i)->outputIndex;
        #endif
    }

    list.outputs.resize(term->numOutputs());
    for (int i=0; i < term->numOutputs(); i++) {
        #if 0
        list.outputs[i].relativeFrame = 0;
        list.outputs[i].index = term->localsIndex + i;
        #endif

        // TEMP
        list.outputs[i].type = InputInstruction::OLD_STYLE_LOCAL;
    }
}

} // namespace circa
