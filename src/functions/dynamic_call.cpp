// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "list_shared.h"

namespace circa {
namespace dynamic_call_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(dynamic_call, "dynamic_call(Function f, List args)")
    {
        Term* function = as_function_pointer(INPUT(0));
        List* inputs = List::checkCast(INPUT(1));

        Term temporaryTerm;
        temporaryTerm.function = function;
        temporaryTerm.type = CALLER->type;

        List* stack = &CONTEXT->stack;
        List* frame = List::cast(stack->append(), 0);

        int numInputs = inputs->length();
        int numOutputs = 1;

        frame->resize(numInputs + numOutputs);
        temporaryTerm.inputIsns.inputs.resize(numInputs);
        temporaryTerm.inputIsns.outputs.resize(numOutputs);

        // Populate input instructions, use our stack frame.
        int frameIndex = 0;
        for (int i=0; i < numInputs; i++) {
            copy(inputs->get(i), frame->get(frameIndex));

            InputInstruction* isn = &temporaryTerm.inputIsns.inputs[i];
            isn->type = InputInstruction::LOCAL;
            isn->data.index = frameIndex;
            isn->data.relativeFrame = 0;
            frameIndex++;
        }

        int outputIndex = frameIndex;

        for (int i=0; i < numOutputs; i++) {
            InputInstruction* isn = &temporaryTerm.inputIsns.outputs[i];
            isn->type = InputInstruction::LOCAL;
            isn->data.index = frameIndex;
            isn->data.relativeFrame = 0;
            frameIndex++;
        }

        // Evaluate
        evaluate_single_term(CONTEXT, &temporaryTerm);

        // Save the stack frame and pop. (the OUTPUT macro isn't valid until
        // we restore the stack to its original size).
        TaggedValue finishedFrame;
        swap(frame, &finishedFrame);
        stack->pop();

        swap(list_get_index(&finishedFrame, outputIndex), OUTPUT);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
