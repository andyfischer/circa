// Copyright 2008 Paul Hodge

#include "circa.h"
#include "function.h"

namespace circa {
namespace subroutine_create_function {

    void evaluate(Term* caller)
    {
        // 0: name (string)
        // 1: inputTypes (list of type)
        // 2: outputType (type)

        as_string(caller->input(0));
        as_type(caller->input(2));

        Function& sub = as_function(caller);
        sub.name = as_string(caller->input(0));
        sub.evaluate = Function::subroutine_call_evaluate;
        sub.inputTypes = as<RefList>(caller->input(1));

        sub.outputType = caller->input(2);

        // Create input placeholders
        for (unsigned int index=0; index < sub.inputTypes.count(); index++) {
            std::string name = get_placeholder_name_for_index(index);
            Term* placeholder = create_value(&sub.subroutineBranch,
                sub.inputTypes[index]);
            sub.subroutineBranch.bindName(placeholder, name);
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "subroutine_create(string,Tuple,Type) : Function");
    }
}
}
