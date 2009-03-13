// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        apply_function(&branch, ASSIGN_FUNC, RefList(desired, subject));
    }

    void setup(Branch& kernel)
    {
        STATEFUL_VALUE_FUNC = import_function(kernel, evaluate, "stateful-value(any) -> any");
        as_function(STATEFUL_VALUE_FUNC).generateTraining = generateTraining;
    }
}
}
