// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #is_first_iteration
     [1] #any_iterations
     [2] #modify_list
     [3] #discard_called
   [1] #rebinds
   [2] iterator
   [3 .. n-2] user's code
   [n-1] #rebinds_for_outer
*/

Branch& get_for_loop_state(Term* forTerm)
{
    return forTerm->input(0)->asBranch();
}

Branch& get_for_loop_iteration_state(Term* forTerm, int index)
{
    return get_for_loop_state(forTerm)[index]->asBranch();
}

Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[1]->asBranch();
}

Term* get_for_loop_iterator(Term* forTerm)
{
    return as_branch(forTerm)[2];
}

Term* get_for_loop_is_first_iteration(Term* forTerm)
{
    return as_branch(forTerm)[0]->asBranch()[0];
}

Term* get_for_loop_any_iterations(Term* forTerm)
{
    return as_branch(forTerm)[0]->asBranch()[1];
}

Term* get_for_loop_modify_list(Term* forTerm)
{
    return as_branch(forTerm)[0]->asBranch()[2];
}

Term* get_for_loop_discard_called(Term* forTerm)
{
    return as_branch(forTerm)[0]->asBranch()[3];
}

Branch& get_for_loop_rebinds_for_outer(Term* forTerm)
{
    Branch& contents = as_branch(forTerm);
    return contents[contents.length()-1]->asBranch();
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch& forContents = as_branch(forTerm);
    Branch& attributes = create_branch(forContents, "#attributes");
    create_bool(attributes, false, "#is_first_iteration");
    create_bool(attributes, false, "#any_iterations");
    create_bool(attributes, false, "#modify_list");
    create_bool(attributes, false, "#discard_called");
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
    std::string listName = forTerm->input(1)->name;
    Branch& outerScope = *forTerm->owningBranch;

    // Rebind any names that are used inside this for loop, using their
    // looped version.
    Branch& rebinds = get_for_loop_rebinds(forTerm);
    {
        rebinds.clear();

        std::vector<std::string> reboundNames;
        list_names_that_this_branch_rebinds(forContents, reboundNames);

        for (unsigned i=0; i < reboundNames.size(); i++) {
            std::string name = reboundNames[i];
            if (name == listName)
                continue;
            Term* outerVersion = find_named(outerScope, name);
            Term* innerVersion = forContents[name];

            apply(rebinds, IF_EXPR_FUNC, RefList(get_for_loop_is_first_iteration(forTerm),
                outerVersion, innerVersion), name);
        }
    }

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
    Branch& rebindsForOuter = create_branch(forContents, "#rebinds_for_outer");
    Term* anyIterations = get_for_loop_any_iterations(forTerm);

    {
        rebindsForOuter.clear();

        std::vector<std::string> reboundNames;
        list_names_that_this_branch_rebinds(forContents, reboundNames);

        for (unsigned i=0; i < reboundNames.size(); i++) {
            std::string name = reboundNames[i];
            if (name == listName)
                continue;
            Term* outerVersion = find_named(outerScope, name);
            Term* innerVersion = forContents[name];

            apply(rebindsForOuter, IF_EXPR_FUNC,
                    RefList(anyIterations, innerVersion, outerVersion), name);
        }
    }

    // Also, possibly rebind the list name.
    if (get_for_loop_modify_list(forTerm)->asBool() && listName != "") {
        create_value(rebindsForOuter, LIST_TYPE, listName);
    }

    expose_all_names(rebindsForOuter, outerScope);
}

void evaluate_for_loop(Term* forTerm)
{
    Term* listTerm = forTerm->input(1);
    Branch& codeBranch = as_branch(forTerm);
    Branch& stateBranch = get_for_loop_state(forTerm);

    // Make sure state has the correct number of iterations

    int numIterations = as_branch(listTerm).length();
    bool modifyList = get_for_loop_modify_list(forTerm)->asBool();
    bool& discardCalled = get_for_loop_discard_called(forTerm)->asBool();

    Term* listOutput = NULL;
    int listOutputWriteHead = 0;

    if (modifyList)
        listOutput = get_for_loop_rebinds_for_outer(forTerm)[listTerm->name];

    resize_list(stateBranch, numIterations, BRANCH_TYPE);

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

    if (numIterations == 0)
        evaluate_branch(get_for_loop_rebinds_for_outer(forTerm));

    for (int i=0; i < numIterations; i++) {
        as_bool(isFirstIteration) = i == 0;
        discardCalled = false;

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

        // Possibly use this value to modify the list
        if (listOutput != NULL && !discardCalled) {
            Term* iteratorResult = codeBranch[iterator->name];
            Branch& listOutputBr = listOutput->asBranch();
            if (listOutputWriteHead >= listOutputBr.length())
                create_value(listOutputBr, iteratorResult->type);
            assign_value(iteratorResult, listOutputBr[listOutputWriteHead++]);
        }
    }

    if (listOutput != NULL && as_branch(listOutput).length() > listOutputWriteHead)
        as_branch(listOutput).shorten(listOutputWriteHead);
}

Term* find_enclosing_for_loop(Term* term)
{
    if (term == NULL)
        return NULL;

    if (term->function == FOR_FUNC)
        return term;

    Branch* branch = term->owningBranch;
    if (branch == NULL)
        return NULL;

    return find_enclosing_for_loop(branch->owningTerm);
}

} // namespace circa
