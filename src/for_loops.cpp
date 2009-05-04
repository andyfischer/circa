// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

Term* get_for_loop_iterator(Term* forTerm)
{
    return get_for_loop_code(forTerm)[0];
}

Branch& get_for_loop_code(Term* forTerm)
{
    return forTerm->state->field("code")->asBranch();
}

Branch& get_for_loop_state(Term* forTerm, int index)
{
    return forTerm->state->field("_state")->field(index)->asBranch();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Term* r = apply(&get_for_loop_code(forTerm), BRANCH_FUNC, RefList(), "#rebound");
    Term* ifi = create_value(&get_for_loop_code(forTerm), BOOL_TYPE, "#is_first_iteration");

    source_set_hidden(r, true);
    source_set_hidden(ifi, true);
}

void setup_for_loop_post_code(Term* forTerm)
{
    // Get a list of rebound names
    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(get_for_loop_code(forTerm), reboundNames);

    Branch& rebound = as_branch(get_for_loop_code(forTerm)["#rebound"]);
    Term* isFirstIteration = get_for_loop_code(forTerm)["#is_first_iteration"];

    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Term* outerVersion = get_for_loop_code(forTerm).outerScope->getNamed(name);
        Term* innerVersion = get_for_loop_code(forTerm)[name];

        apply(&rebound, IF_EXPR_FUNC, RefList(isFirstIteration, outerVersion, innerVersion));
    }
}

void evaluate_for_loop(Term* forTerm, Term* listTerm)
{
    Branch& codeBranch = get_for_loop_code(forTerm);
    Branch& stateBranch = forTerm->state->field("_state")->asBranch();

    int numIterations = as_branch(listTerm).numTerms();

    resize_list(stateBranch, as_branch(listTerm).numTerms(), BRANCH_TYPE);

    // Set up state: duplicate code once for each iteration. This should
    // be optimized.
    for (int i=0; i < numIterations; i++) {
        // Set up state: duplicate code once for each iteration. This should
        // be optimized.

        Branch& iterationBranch = get_for_loop_state(forTerm, i);
        
        if (iterationBranch.numTerms() == 0)
            duplicate_branch(codeBranch, iterationBranch);
    }

    for (int i=0; i < numIterations; i++) {
        Branch& iterationBranch = get_for_loop_state(forTerm, i);

        if (iterationBranch.contains("#is_first_iteration"))
            as_bool(iterationBranch["#is_first_iteration"]) = i == 0;

        // Inject iterator value
        Term* iterator = iterationBranch[0];
        if (!value_fits_type(listTerm->field(i), iterator->type)) {
            error_occured(forTerm, "Internal error in evaluate_for_loop: can't assign this element to iterator");
            return;
        }
        assign_value(listTerm->field(i), iterator);

        // Evaluate
        evaluate_branch(iterationBranch);
    }
}

} // namespace circa
