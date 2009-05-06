// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(Term* caller)
    {
        // check if we should initialize our value
        if ((caller->numInputs() > 0) &&
                // FIXME: Shouldn't use a property for this
                !caller->boolPropOptional("initialized", false)) {
            assign_value(caller->input(0), caller);
            caller->boolProp("initialized") = true;
        }
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "state ";
        result << term->name;

        // check for initial value
        if (term->numInputs() > 0) {
            result << " = ";
            result << get_source_of_input(term, 0);
        }
        return result.str();
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        apply(&branch, ASSIGN_FUNC, RefList(desired, subject));
    }

    void setup(Branch& kernel)
    {
        STATEFUL_VALUE_FUNC = import_function(kernel, evaluate, "stateful_value(any) -> any");
        as_function(STATEFUL_VALUE_FUNC).generateTraining = generateTraining;
        as_function(STATEFUL_VALUE_FUNC).toSourceString = toSourceString;
    }
}
}
