// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(Term* caller) {
        as_branch(caller).clear();

        for (unsigned int i=0; i < caller->inputs.count(); i++) {
            create_duplicate(&as_branch(caller), caller->input(i));
        }
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate,
                "function list(any) -> List");
        as_function(LIST_FUNC).variableArgs = true;
        as_function(LIST_FUNC).pureFunction = true;
    }
}
}
