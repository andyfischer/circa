// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "list.h"

namespace circa {
namespace list_pack_function {

    void evaluate(Term* caller) {
        as_list(caller).clear();

        for (unsigned int i=0; i < caller->inputs.count(); i++) {
            as_list(caller).append(caller->inputs[i]);
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function list-pack(any) -> List");
        as_function(main_func).variableArgs = true;
        as_function(main_func).pureFunction = true;
        kernel.bindName(main_func, "list");
    }
}
}
