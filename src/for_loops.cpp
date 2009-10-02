// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

/* Organization of for loop contents:
   [0] #state
   [1] #is_first_iteration
   [2] #rebinds
   [3] iterator
   [4 .. n-1] user's code
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

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch& forContents = as_branch(forTerm);
    create_list(forContents, "#state");
    create_value(forContents, BOOL_TYPE, "#is_first_iteration");
    create_branch(forContents, "#rebinds");
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch& forContents = as_branch(forTerm);
    
    // Get a list of rebound names
    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(forContents, reboundNames);

    Branch& rebinds = get_for_loop_rebinds(forTerm);
    Term* isFirstIteration = get_for_loop_is_first_iteration(forTerm);

    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Branch& outerScope = *forTerm->owningBranch;
        Term* outerVersion = find_named(outerScope, name);
        Term* innerVersion = forContents[name];

        Term* ifexpr = apply(rebinds, IF_EXPR_FUNC,
                RefList(isFirstIteration, outerVersion, innerVersion));

        remap_pointers(forContents, outerVersion, ifexpr);

        // undo remap
        ifexpr->inputs[1] = outerVersion;

        // Bind inner version to outer scope
        Term* copy = apply(outerScope, COPY_FUNC, RefList(innerVersion), name);
        set_source_hidden(copy, true);
    }
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

    for (int i=0; i < numIterations; i++) {

        Term* isFirstIteration = get_for_loop_is_first_iteration(forTerm);
        assert(isFirstIteration->name == "#is_first_iteration");

        as_bool(isFirstIteration) = i == 0;

        // Inject iterator value
        Term* iterator = get_for_loop_iterator(forTerm);
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
