// Copyright 2008 Paul Hodge

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


    void evaluate(Term* caller)
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
                break;
            }
        }

        // Update the #joining branch
        assert(contents[contents.length()-1]->name == "#joining");
        Branch& joining = as_branch(contents[contents.length()-1]);
        joining["#satisfiedIndex"]->asInt() = satisfiedIndex;
        evaluate_branch(joining, caller);
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;

        result << "if";
        result << " " << get_source_of_input(term->field(0), 0) << std::endl;

        Branch& contents = term->asBranch();

        Branch& positiveBranch = contents[0]->asBranch();

        result << get_branch_source(positiveBranch);

        Term* elseTerm = contents["else"];
        if (!is_hidden(elseTerm)) {
            result << elseTerm->stringPropOptional("syntaxHints:preWhitespace", "");
            result << "else";
            Branch& elseBranch = contents["else"]->asBranch();
            result << get_branch_source(elseBranch);
        }

        result << term->stringPropOptional("syntaxHints:whitespaceBeforeEnd", "");
        result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate, "if_block() : Branch");
        function_get_to_source_string(IF_BLOCK_FUNC) = toSourceString;
    }
}
}
