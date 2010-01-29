// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace average_function {

    void evaluate(EvalContext*, Term* caller)
    {
        if (caller->numInputs() == 0) {
            set_float(caller, 0);
            return;
        }

        float sum = 0;
        for (int i=0; i < caller->numInputs(); i++)
            sum += float_input(caller,i);

        set_float(caller, sum / caller->numInputs());
    }

    void setup(Branch& kernel)
    {
        AVERAGE_FUNC = import_function(kernel, evaluate, "average(number...) -> number;"
                "'Returns the average of all inputs.' end");
    }
}
}
