// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_block_function {

    // The format of if_block is as follows:
    //
    // N = branch length
    //
    // {
    //   [0] if(cond0) : Branch
    //   [1] if(cond1) : Branch
    //   ...
    //   [N-2] branch()  (this corresponds to the 'else' block)
    //   [N-1] #joining = branch() 
    //

    void evaluate(EvalContext*, Term* caller)
    {
        // Find the first if() call whose condition is true
        Branch& contents = as_branch(caller);
        int satisfiedIndex = 0;
        for (int i=0; i < contents.length()-1; i++) {
            Term* call = contents[i];

            bool satisfied = false;

            if (call->function == BRANCH_FUNC)
                satisfied = true;
            else {
                Term* cond = call->input(0);
                if (cond->asBool())
                    satisfied = true;
            }

            if (satisfied) {
                evaluate_term(call);
                satisfiedIndex = i;

                if (call->hasError()) {
                    nested_error_occurred(caller);
                    return;
                }
                break;
            }
        }

        // Update the #joining branch
        assert(contents[contents.length()-1]->name == "#joining");
        Branch& joining = as_branch(contents[contents.length()-1]);
        set_int(joining["#satisfiedIndex"], satisfiedIndex);
        evaluate_branch(joining, caller);
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;

        Branch& contents = as_branch(term);

        for (int branch_index=0; branch_index < contents.length(); branch_index++) {
            Term* branch_term = contents[branch_index];

            if (is_hidden(branch_term))
                continue;

            result << branch_term->stringPropOptional("syntax:preWhitespace", "");

            if (branch_index == 0) {
                result << "if ";
                result << get_source_of_input(branch_term, 0);
            } else if (branch_index < (contents.length()-2)) {
                result << "elif ";
                result << get_source_of_input(branch_term, 0);
            }
            else
                result << "else";

            result << get_branch_source(as_branch(branch_term));
        }

        result << term->stringPropOptional("syntax:whitespaceBeforeEnd", "");
        result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate, "if_block() -> Code");
        function_t::get_to_source_string(IF_BLOCK_FUNC) = toSourceString;
        function_t::set_exposed_name_path(IF_BLOCK_FUNC, "#joining");
    }
}
}
