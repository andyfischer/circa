// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

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

void update_if_block_joining_branch(Term* ifCall)
{
    Branch& contents = ifCall->asBranch();

    // Create the joining contents if necessary
    if (!contents.contains("#joining"))
        create_branch(contents, "#joining");

    Branch& joining = contents["#joining"]->asBranch();
    joining.clear();

    // This is used later.
    Term* satisfiedIndex = create_int(joining, 0, "#satisfiedIndex");

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    {
        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Branch& branch = contents[branch_index]->asBranch();

            TermNamespace::const_iterator it;
            for (it = branch.names.begin(); it != branch.names.end(); ++it)
                boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // Ignore hidden names
        if ((name[0] == '#') && (name != "#out")) {
            boundNames.erase(it++);
            continue;
        }

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_named(*outerScope, name) != NULL;

        int numberOfBranchesWithThisName = 0;
        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            if (contents[branch_index]->asBranch().contains(name))
                numberOfBranchesWithThisName++;
            else
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        // Make a list where we find the corresponding term for this name in each branch.
        Term* selections = apply(joining, BRANCH_FUNC, RefList());
        Branch& selectionsBranch = as_branch(selections);

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Branch& branch = contents[branch_index]->asBranch();

            Term* selection = NULL;
            if (branch.contains(name))
                selection = branch[name];
            else
                selection = find_named(*outerScope, name);

            apply(selectionsBranch, COPY_FUNC, RefList(selection));
        }

        apply(joining, GET_INDEX_FUNC, RefList(selections, satisfiedIndex), name);
    }

    // Expose all names in 'joining' branch.
    expose_all_names(joining, *outerScope);
}

Branch* get_if_condition_block(Term* ifCall, int index)
{
    assert(ifCall->function = IF_BLOCK_FUNC);
    Branch& callContents = as_branch(ifCall);
    assert(index < callContents.length());
    return (&as_branch(callContents[index]));
}

Branch* get_if_block_else_block(Term* ifCall)
{
    assert(ifCall->function = IF_BLOCK_FUNC);
    Branch& callContents = as_branch(ifCall);
    assert(callContents.length() >= 2);
    return (&as_branch(callContents[callContents.length()-2]));
}

List* get_if_block_state(Term* ifCall)
{
    Term* term = ifCall->input(0);
    if (term == NULL)
        return NULL;
    return List::checkCast(term);
}

bool if_block_contains_state(Term* ifCall)
{
    Branch& contents = as_branch(ifCall);
    for (int cond=0; cond < contents.length(); cond++) {
        Branch& condContents = as_branch(contents[cond]);
        for (int i=0; i < condContents.length(); i++) {
            if (is_stateful(condContents[i]))
                return true;
        }
    }
    return false;
}

void evaluate_if_block(EvalContext* cxt, Term* caller)
{
    Branch& contents = as_branch(caller);
    List* state = get_if_block_state(caller);

    if (state != NULL) {
        int numElements = contents.length() - 1;
        int actualElements = state->numElements();
        state->resize(numElements);
        for (int i=actualElements; i < numElements; i++)
            make_branch(state->get(i));
    }

    // Find the first if() call whose condition is true
    int satisfiedIndex = 0;
    for (int i=0; i < contents.length()-1; i++) {
        Term* call = contents[i];

        TaggedValue* stateElement = NULL;
        if (state != NULL)
            stateElement = state->get(i);

        bool satisfied = false;

        if (call->function == BRANCH_FUNC) {
            satisfied = true;
        } else {
            Term* cond = call->input(0);
            if (cond->asBool())
                satisfied = true;
        }

        if (satisfied) {
            // Load state, if it's found
            if (stateElement != NULL)
                load_state_into_branch(as_branch(stateElement), as_branch(call));

            evaluate_term(cxt, call);
            satisfiedIndex = i;

            if (stateElement != NULL)
                persist_state_from_branch(as_branch(call), as_branch(stateElement));

            break;

        } else {
            // This condition wasn't executed, reset state.
            if (stateElement != NULL) {
                as_branch(stateElement).clear();
                reset_state(as_branch(call));
            }
        }
    }

    // For any conditions after the successful one, reset state.
    for (int i=satisfiedIndex+1; i < contents.length()-1; i++) {
        TaggedValue* stateElement = NULL;
        if (state != NULL) {
            stateElement = state->get(i);
        }
        if (stateElement != NULL) {
            as_branch(stateElement).clear();
            Term* call = contents[i];
            reset_state(as_branch(call));
        }
    }

    // Update the #joining branch
    assert(contents[contents.length()-1]->name == "#joining");
    Branch& joining = as_branch(contents[contents.length()-1]);
    set_int(joining["#satisfiedIndex"], satisfiedIndex);
    evaluate_branch(cxt, joining);
}

} // namespace circa
