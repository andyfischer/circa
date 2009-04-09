// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(Term* caller) {
        as_branch(caller).clear();

        for (unsigned int i=0; i < caller->inputs.count(); i++) {
            create_duplicate(&as_branch(caller), caller->input(i));
        }
    }

    std::string toSourceString(Term* caller) {
        std::stringstream out;
        if (caller->name != "")
            out << caller->name << " = ";
        out << "[";
        for (int i=0; i < caller->numInputs(); i++)
            out << get_source_of_input(caller, i);
        out << "]";
        return out.str();
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate,
                "function list(any) -> List");
        as_function(LIST_FUNC).variableArgs = true;
        as_function(LIST_FUNC).pureFunction = true;
        as_function(LIST_FUNC).toSourceString = toSourceString;
    }
}
}
