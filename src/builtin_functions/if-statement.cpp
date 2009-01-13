// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace if_statement_function {

    /*
    type State
    {
      Branch positiveBranch
      Branch negativeBranch
      Branch joiningTerms
    }
    */

    void evaluate(Term* caller)
    {
        bool condition = as_bool(caller->input(0));

        if (condition)
            evaluate_branch(as_branch(get_field(caller->state, 0)));
        else
            evaluate_branch(as_branch(get_field(caller->state, 1)));

        evaluate_branch(as_branch(get_field(caller->state, 2)));
    }

    void setup(Branch& kernel)
    {

        Term* main_func = import_function(kernel, evaluate,
                "function if-statement(bool)");
        as_function(main_func).pureFunction = false;

        kernel.eval(
            "if-statement::State = create-compound-type('if-statement::State')");
        kernel.eval("compound-type-append-field(@if-statement::State, Branch, 'positiveBranch')");
        kernel.eval("compound-type-append-field(@if-statement::State, Branch, 'negativeBranch')");
        kernel.eval("compound-type-append-field(@if-statement::State, Branch, 'joiningTerms')");
        as_function(main_func).stateType = kernel["if-statement::State"];
    }
}
}
