// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace for_function {

    void evaluate(Term* caller)
    {
        Branch& series = as_branch(caller->input(0));

        std::string iteratorName = as_string(caller->state->field("iteratorName"));

        evaluate_branch(as_branch(caller->state->field("inputs")));

        Branch& contents = as_branch(caller->state->field("contents"));

        Term* iterator = contents[iteratorName];
        assert(iterator != NULL);

        for (int i=0; i < series.numTerms(); i++) {

            assign_value(series[i], iterator);
            evaluate_branch(contents);
        }
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "for ";
        result << as_string(term->state->field("iteratorName"));
        result << " in";
        result << get_term_source(term->input(0));
        result << "\n";
        result << get_branch_source(as_branch(term->state->field("contents")));
        result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate, "for(List)");
        as_function(FOR_FUNC).pureFunction = true;
        as_function(FOR_FUNC).toSourceString = toSourceString;
        as_function(FOR_FUNC).stateType = create_type(&kernel,
            "type For::State { string iteratorName, Branch contents, Branch inputs }");
    }
}
} // namespace circa
