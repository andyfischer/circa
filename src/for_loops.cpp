// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

Term* get_for_loop_iterator(Term* forTerm)
{
    return get_for_loop_code(forTerm)[0];
}

Branch& get_for_loop_code(Term* forTerm)
{
    return forTerm->field("code")->asBranch();
}

Branch& get_for_loop_state(Term* forTerm, int index)
{
    return forTerm->field("_state")->field(index)->asBranch();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    create_branch(&get_for_loop_code(forTerm), "#rebound");
    create_value(&get_for_loop_code(forTerm), BOOL_TYPE, "#is_first_iteration");
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
        Branch& outerScope = *forTerm->owningBranch;
        Term* outerVersion = outerScope.getNamed(name);
        Term* innerVersion = get_for_loop_code(forTerm)[name];

        Term* ifexpr = apply(&rebound, IF_EXPR_FUNC,
                RefList(isFirstIteration, outerVersion, innerVersion));

        remap_pointers(get_for_loop_code(forTerm), outerVersion, ifexpr);

        // undo remap
        ifexpr->inputs[1] = outerVersion;

        // Bind inner version to outer scope
        apply(&outerScope, COPY_FUNC, RefList(innerVersion), name);
    }
}

void evaluate_for_loop(Term* forTerm, Term* listTerm)
{
    Branch& codeBranch = get_for_loop_code(forTerm);
    Branch& stateBranch = forTerm->field("_state")->asBranch();

    // Make sure state has the correct number of iterations

    int numIterations = as_branch(listTerm).length();

    resize_list(stateBranch, as_branch(listTerm).length(), BRANCH_TYPE);

    // Initialize state for any uninitialized slots
    for (int i=0; i < numIterations; i++) {

        Branch& iterationBranch = get_for_loop_state(forTerm, i);
        
        if (iterationBranch.length() == 0) {
            get_type_from_branches_stateful_terms(codeBranch, iterationBranch);
        }
    }

    for (int i=0; i < numIterations; i++) {

        as_bool(codeBranch["#is_first_iteration"]) = i == 0;

        // Inject iterator value
        Term* iterator = codeBranch[0];
        if (!value_fits_type(listTerm->field(i), iterator->type)) {
            error_occured(forTerm, "Internal error in evaluate_for_loop: can't assign this element to iterator");
            return;
        }
        assign_value(listTerm->field(i), iterator);

        // Inject stateful terms
        load_state_into_branch(stateBranch[i]->asBranch(), codeBranch);

        // Evaluate
        evaluate_branch(codeBranch);

        // Persist stateful terms
        persist_state_from_branch(codeBranch, stateBranch[i]->asBranch());
    }
}

} // namespace circa
