// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace average_function {

    void evaluate(Term* caller)
    {
        if (caller->numInputs() == 0) {
            as_float(caller) = 0;
            return;
        }

        float sum = 0;
        for (int i=0; i < caller->numInputs(); i++)
            sum += to_float(caller->input(i));

        as_float(caller) = sum / caller->numInputs();
    }

    void setup(Branch& kernel)
    {
        AVERAGE_FUNC = import_function(kernel, evaluate, "average(float...) : float");
        as_function(AVERAGE_FUNC).pureFunction = true;
    }
}
}
