// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace unknown_type_function {

    void evaluate(Term* caller)
    {
        setup_empty_type(as_type(caller));
    }

    void setup(Branch& kernel)
    {
        UNKNOWN_TYPE_FUNC = import_function(kernel, evaluate, "function unknown-type() -> Type");
        as_function(UNKNOWN_TYPE_FUNC).stateType = STRING_TYPE;
        as_function(UNKNOWN_TYPE_FUNC).pureFunction = false;
        as_function(UNKNOWN_TYPE_FUNC).hasSideEffects = true;
        as_function(UNKNOWN_TYPE_FUNC).variableArgs = true;
    }
}
}
