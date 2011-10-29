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
    Function::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;

    Function* attrs = get_function_attrs(term->function);

    if (attrs == NULL)
        return 1;
    
    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term);

    // Default behavior, if Function was found.
    return attrs->outputCount;
}

int get_locals_count(Branch* branch)
{
    return branch->length();
}

void update_output_count(Term* term)
{
    term->outputCount = get_output_count(term);
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
