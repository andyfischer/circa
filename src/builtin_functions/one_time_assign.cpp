// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace one_time_assign_function {

    void evaluate(Term* caller)
    {
        bool &assigned = caller->input(0)->asBool();

        if (!assigned) {
            assign_value(caller->input(1), caller);
            assigned = true;
        }
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(1)->type;
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "state " << term->name << " = " << get_source_of_input(term, 1);
        return result.str();
    }

    void setup(Branch& kernel)
    {
        ONE_TIME_ASSIGN_FUNC = import_function(kernel, evaluate,
                "one_time_assign(state bool, any) : any");
        function_get_specialize_type(ONE_TIME_ASSIGN_FUNC) = specializeType;
        function_get_to_source_string(ONE_TIME_ASSIGN_FUNC) = toSourceString;
    }
}
}
