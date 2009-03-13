// Copyright 2008 Andrew Fischer

#include "circa.h"

#include "math.h"

namespace circa {
namespace cos_function {

    void evaluate(Term* caller)
    {
        float input = as_float(caller->input(0));
        as_float(caller) = cos(input);
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        Term* inputDesired = float_value(branch, acos(as_float(desired)));
        generate_training(branch, subject->input(0), inputDesired);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function cos(float) -> float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).generateTraining = generateTraining;
    }
}
}
