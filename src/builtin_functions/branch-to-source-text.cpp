// Copyright 2008 Paul Hodge

#include "branch.h"
#include "circa.h"

namespace circa {
namespace branch_to_source_text_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as<Branch>(caller->input(0));
        std::stringstream result;

        for (int i=0; i < branch.numTerms(); i++) {

            Term* term = branch[i];

            Branch workspace;
            std::string source = as_string(eval_function(workspace,
                    "term-to-source-line", ReferenceList(term)));

            result << source << "\n";
        }

        as_string(caller) = result.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function branch-to-source-text(Branch) -> string");
        as_function(main_func).pureFunction = true;
    }
}
}
