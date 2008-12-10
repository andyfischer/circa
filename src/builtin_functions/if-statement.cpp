// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_statement_function {

    struct State {
        Branch positiveBranch;
        Branch negativeBranch;
    };

    void evaluate(Term* caller)
    {
        bool condition = as_bool(caller->input(0));
        State &state = as<State>(caller->state);

        if (condition)
            evaluate_branch(state.positiveBranch);
        else
            evaluate_branch(state.negativeBranch);
    }

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
    }

    void setup(Branch& kernel)
    {
        Term* stateType = quick_create_cpp_type<State>(kernel, "if-statement::State");
        Term* main_func = import_c_function(kernel, evaluate,
                "function if-statement(bool)");
        as_function(main_func).pureFunction = false;
        as_function(main_func).stateType = stateType;

        if_statement_get_branch_function::setup(kernel);
    }
}
}
