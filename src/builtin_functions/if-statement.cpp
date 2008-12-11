// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace if_statement_function {

    /*
    type State
    {
      Branch positiveBranch
      Branch negativeBranch
    }
    */

    void evaluate(Term* caller)
    {
        bool condition = as_bool(caller->input(0));

        if (condition)
            evaluate_branch(as_branch(get_field(caller->state, 0)));
        else
            evaluate_branch(as_branch(get_field(caller->state, 1)));
    }

    void setup(Branch& kernel)
    {

        Term* main_func = import_c_function(kernel, evaluate,
                "function if-statement(bool)");
        as_function(main_func).pureFunction = false;

        eval_statement(kernel,
                "if-statement::State = create-compound-type('if-statement::State')");
        eval_statement(kernel,
                "compound-type-append-field(@if-statement::State, Branch, 'positiveBranch')");
        as_function(main_func).stateType = eval_statement(kernel,
                "compound-type-append-field(@if-statement::State, Branch, 'negativeBranch')");
    }
}
}
