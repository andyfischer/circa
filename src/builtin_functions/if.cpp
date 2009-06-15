// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_function {

    void evaluate(Term* caller)
    {
        Branch& contents = caller->asBranch();
        bool cond = as_bool(caller->input(0));

        if (cond) {
            Branch& branch = contents["if"]->asBranch();
            evaluate_branch(branch);
        } else if (contents.contains("else")) {
            Branch& branch = contents["else"]->asBranch();
            evaluate_branch(branch);
        }

        evaluate_branch(contents["#joining"]->asBranch());
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;

        result << "if";
        result << " " << get_source_of_input(term, 0) << std::endl;

        Branch& contents = term->asBranch();

        Branch& positiveBranch = contents["if"]->asBranch();

        result << get_branch_source(positiveBranch);

        if (contents.contains("else")) {
            result << term->stringPropOptional("syntaxHints:whitespaceBeforeElse", "");
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
        IF_FUNC = import_function(kernel, evaluate, "if(bool) : Branch");
        function_get_to_source_string(IF_FUNC) = toSourceString;
    }
}
}
