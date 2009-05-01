// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace for_function {

    void evaluate(Term* caller)
    {
        Branch& series = as_branch(caller->input(0));

        std::string iteratorName = as_string(caller->state->field("iteratorName"));

        evaluate_branch(as_branch(caller->state->field("inputs")));

        Branch& contents = as_branch(caller->state->field("contents"));

        Term* iterator = contents.findFirstBinding(iteratorName);
        assert(iterator != NULL);

        Term* iteratorResult = contents.findLastBinding(iteratorName);

        as_branch(caller->state->field("results")).clear();

        for (int i=0; i < series.numTerms(); i++) {
            if (!value_fits_type(series[i], iterator->type)) {
                error_occured(caller, "Internal error in for(): can't assign this element to iterator");
                return;
            }

            assign_value(series[i], iterator);
            evaluate_branch(contents);

            // Save the iterator result, if we are doing a mapping
            if (iterator != iteratorResult) {
                list_t::append(as_branch(caller->state->field("results")), iteratorResult);
            }
        }
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "for ";
        result << as_string(term->state->field("iteratorName"));
        result << " in ";
        result << get_source_of_input(term,0);
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
            "type for__state { "
                "string iteratorName, "
                "Branch contents, "
                "Branch inputs, "
                "Branch results "
                "}");
    }
}
} // namespace circa
