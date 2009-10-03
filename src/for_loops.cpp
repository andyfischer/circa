// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

/* Organization of for loop contents:
   [0] #state
   [1] #is_first_iteration
   [2] #rebinds
   [3] iterator
   [4 .. n-3] user's code
   [n-2] #any_iterations
   [n-1] #rebinds_for_outer
*/


Branch& get_for_loop_state(Term* forTerm)
{
    return as_branch(forTerm)[0]->asBranch();
}

Branch& get_for_loop_iteration_state(Term* forTerm, int index)
{
    return get_for_loop_state(forTerm)[index]->asBranch();
}

Term* get_for_loop_is_first_iteration(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[1];
}

Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[2]->asBranch();
}

Term* get_for_loop_iterator(Term* forTerm)
{
    return as_branch(forTerm)[3];
}

Term* get_for_loop_any_iterations(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[contents.length()-2];
}

Branch& get_for_loop_rebinds_for_outer(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[contents.length()-1]->asBranch();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch& forContents = as_branch(forTerm);
    create_list(forContents, "#state");
    create_value(forContents, BOOL_TYPE, "#is_first_iteration");
    create_branch(forContents, "#rebinds");
}

void create_rebind_branch(Branch& rebinds, Branch& source, Term* rebindCondition, bool outsidePositive)
{
    rebinds.clear();

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(source, reboundNames);

    Branch& outerScope = *source.owningTerm->owningBranch;
    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Term* outerVersion = find_named(outerScope, name);
        Term* innerVersion = source[name];

        Term* pos = outsidePositive ? outerVersion : innerVersion;
        Term* neg = outsidePositive ? innerVersion : outerVersion ;
        apply(rebinds, IF_EXPR_FUNC, RefList(rebindCondition, pos, neg), name);
    }
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch& forContents = as_branch(forTerm);

    // Rebind any names that are used inside this for loop, using their
    // looped version.
    Branch& rebinds = get_for_loop_rebinds(forTerm);
    create_rebind_branch(rebinds, forContents, get_for_loop_is_first_iteration(forTerm), true);

    Branch& outerScope = *forContents.owningTerm->owningBranch;

    // Rewrite code to use these rebound versions
    for (int i=0; i < rebinds.length(); i++) {
        Term* ifexpr = rebinds[i];
        Term* outerVersion = ifexpr->input(1);

        remap_pointers(forContents, outerVersion, ifexpr);

        // undo remap to the arguments of if_expr()
        ifexpr->inputs[1] = outerVersion;
    }

    // Now do another rebinding, this one has copies that we expose outside of this branch.
    // If the for loop isn't executed at all then we use outer versions, similar to an if()
    // rebinding.
    Term* anyIterations = create_value(forContents, BOOL_TYPE, "#any_iterations");
    Branch& rebindsForOuter = create_branch(forContents, "#rebinds_for_outer");

    create_rebind_branch(rebindsForOuter, forContents, anyIterations, false);
    expose_all_names(rebindsForOuter, outerScope);
}

void evaluate_for_loop(Term* forTerm, Term* listTerm)
{
    Branch& codeBranch = as_branch(forTerm);
    Branch& stateBranch = get_for_loop_state(forTerm);

    // Make sure state has the correct number of iterations

    int numIterations = as_branch(listTerm).length();

    resize_list(stateBranch, as_branch(listTerm).length(), BRANCH_TYPE);

    // Initialize state for any uninitialized slots
    for (int i=0; i < numIterations; i++) {

        Branch& iterationBranch = get_for_loop_iteration_state(forTerm, i);
        
        if (iterationBranch.length() == 0) {
            get_type_from_branches_stateful_terms(codeBranch, iterationBranch);
        }
    }

    Term* isFirstIteration = get_for_loop_is_first_iteration(forTerm);
    assert(isFirstIteration->name == "#is_first_iteration");
    Term* iterator = get_for_loop_iterator(forTerm);
    get_for_loop_any_iterations(forTerm)->asBool() = numIterations > 0;

    if (numIterations == 0) {
        evaluate_branch(get_for_loop_rebinds_for_outer(forTerm));
    }

    for (int i=0; i < numIterations; i++) {
        as_bool(isFirstIteration) = i == 0;

        // Inject iterator value
        if (!value_fits_type(listTerm->asBranch()[i], iterator->type)) {
            error_occurred(forTerm, "Internal error in evaluate_for_loop: can't assign this element to iterator");
            return;
        }
        assign_value(listTerm->asBranch()[i], iterator);

        // Inject stateful terms
        load_state_into_branch(stateBranch[i]->asBranch(), codeBranch);

        // Evaluate
        Term errorListener;
        evaluate_branch(codeBranch, &errorListener);

        if (errorListener.hasError()) {
            nested_error_occurred(forTerm);
            break;
        }

        // Persist stateful terms
        persist_state_from_branch(codeBranch, stateBranch[i]->asBranch());
    }
}

} // namespace circa
