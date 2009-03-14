// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace sin_function {

    void evaluate(Term* caller)
    {
        float input = as_float(caller->input(0));
        as_float(caller) = sin(input);
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        std::cout << "sin gt:" << std::endl;
        std::cout << "subject = " << as_float(subject) << std::endl;
        std::cout << "desired = " << as_float(desired) << std::endl;
        Term* inputDesired = float_value(branch, asin(as_float(desired)));
        std::cout << "desired_in = " << as_float(inputDesired) << std::endl;
        generate_training(branch, subject->input(0), inputDesired);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function sin(float) -> float");
        as_function(main_func).pureFunction = true;
        as_function(main_func).generateTraining = generateTraining;
    }
}
}
