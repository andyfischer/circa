// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_statement_function {

    /*
    type State
    {
      Branch positiveBranch
      Branch negative branch
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

    /*
    namespace if_statement_get_branch_function {

        // This function is hopefully temporary, it exposes the branches inside
        // the state of if-statement

        void evaluate(Term* caller)
        {
            State &state = as<State>(caller->input(0)->state);
            bool condition = as_bool(caller->input(1));

            Branch* result = NULL;
            if (condition)
                result = &state.positiveBranch;
            else
                result = &state.negativeBranch;

            as<Branch*>(caller) = result;
        }

        void setup(Branch& kernel)
        {
            Term* main_func = import_c_function(kernel, evaluate,
                    "function if-statement-get-branch(any, bool) -> BranchPtr");
            as_function(main_func).pureFunction = true;
            as_function(main_func).setInputMeta(0, true);
        }
    }*/

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
