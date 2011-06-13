// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct InputInstruction
{
    // InputInstruction
    //
    // For each term, each input has InputInstruction data which records where the
    // input data can be found. This is precomputed in update_input_instructions.
    
    enum Type {
        // EMPTY means that there is no input value.
        EMPTY=0,

        // GLOBAL means to use the input term's global value.
        GLOBAL,
        
        // LOCAL means to use the local value on the stack. LocalData will be populated
        // with details on where to look on the stack.
        LOCAL,

        // LOCAL_CONSUME is similar to LOCAL, but the stack value is swapped in (consumed)
        // instead of copied.
        LOCAL_CONSUME,

        OLD_STYLE_LOCAL
    };

    struct LocalData {
        int relativeFrame;
        int index;
    };

    Type type;
    LocalData data;
};

struct InputInstructionList
{
    std::vector<InputInstruction> inputs;
    std::vector<InputInstruction> outputs;
};

} // namespace circa
