// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "function.h"

namespace circa {
namespace function_name_input_function {

    void evaluate(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        int index = as_int(caller->input(1));
        std::string name = as_string(caller->input(2));
        Function& sub = as_function(caller);

        if (index < 0) {
            error_occured(caller, "index must be >= 0");
            return;
        }

        if ((unsigned int) index >= sub.inputTypes.count()) {
            error_occured(caller, "index out of bounds");
            return;
        }

        Term* inputPlaceholder =
            sub.subroutineBranch.getNamed(get_placeholder_name_for_index(index));

        assert(inputPlaceholder != NULL);

        // remove the name on this term, so that this new name will stick
        inputPlaceholder->name = "";
        sub.subroutineBranch.bindName(inputPlaceholder, name);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
                "function-name-input(Function,int,string) -> Function");
    }
}
}
