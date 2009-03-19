// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "builtins.h"
#include "importing.h"
#include "type.h"

namespace circa {
namespace comment_function {

    /*
    type State
    {
      string str
    }
    */

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function comment()");
        as_function(main_func).pureFunction = true;

        Term* stateT = create_compound_type(kernel, "comment::State");
        as_type(stateT).addField(STRING_TYPE, "str");

        as_function(main_func).stateType = kernel["comment::State"];

        COMMENT_FUNC = main_func;
    }
}
}
