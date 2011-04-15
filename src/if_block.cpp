// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "ref_list.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"

#include "if_block.h"

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
    Branch& contents = ifCall->nestedContents;

    // Create the joining contents if necessary
    if (!contents.contains("#joining"))
        create_branch(contents, "#joining");

    Branch& joining = contents["#joining"]->nestedContents;
    clear_branch(&joining);

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
        Term* term = contents[branch_index];
        Branch& branch = term->nestedContents;

        TermNamespace::const_iterator it;
        for (it = branch.names.begin(); it != branch.names.end(); ++it) {
            std::string const& name = it->first;

            // Ignore empty or hidden names
            if (name == "" || name[0] == '#') {
                continue;
            }

            boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    ca_assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_named(*outerScope, name) != NULL;

        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Branch& branch = contents[branch_index]->nestedContents;
            if (!branch.contains(name))
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    int numBranches = contents.length() - 1;

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        RefList inputs;
        inputs.resize(numBranches);

        Term* outerVersion = get_named_at(ifCall, name);

        for (int i=0; i < numBranches; i++) {
            Term* local = contents[i]->nestedContents[name];
            inputs[i] = local == NULL ? outerVersion : local;
        }

        apply(joining, JOIN_FUNC, inputs, name);
    }
}

Branch* get_if_condition_block(Term* ifCall, int index)
{
    ca_assert(ifCall->function == IF_BLOCK_FUNC);
    Branch& callContents = ifCall->nestedContents;
    ca_assert(index < callContents.length());
    return &(callContents[index]->nestedContents);
}

Branch* get_if_block_else_block(Term* ifCall)
{
    ca_assert(ifCall->function == IF_BLOCK_FUNC);
    Branch& callContents = ifCall->nestedContents;
    ca_assert(callContents.length() >= 2);
    return &(callContents[callContents.length()-2]->nestedContents);
}

Branch* get_if_block_joining_branch(Term* ifCall)
{
    Term* joiningBranch = ifCall->nestedContents["#joining"];
    if (joiningBranch == NULL)
        return NULL;
    return &joiningBranch->nestedContents;
}

CA_FUNCTION(evaluate_if_block)
{
    EvalContext* context = CONTEXT;
    Branch& contents = CALLER->nestedContents;
    bool useState = has_any_inlined_state(contents);

    int numBranches = contents.length() - 1;
    int acceptedBranchIndex = 0;
    Branch* acceptedBranch = NULL;

    // Keep track of whether the accepted branch was empty; this is for deciding whether
    // to reset state.
    bool acceptedBranchWasEmpty = true;

    TaggedValue localState;
    TaggedValue prevScopeState;
    List* state = NULL;
    if (useState) {
        swap(&prevScopeState, &context->currentScopeState);
        fetch_state_container(CALLER, &prevScopeState, &localState);
        state = List::lazyCast(&localState);
        state->resize(numBranches);
    }

    for (int branchIndex=0; branchIndex < numBranches; branchIndex++) {
        Term* branch = contents[branchIndex];

        //std::cout << "checking: " << get_term_to_string_extended(branch) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        if (branch->numInputs() == 0 || as_bool(get_input(branch, 0))) {

            acceptedBranch = &branch->nestedContents;

            start_using(*acceptedBranch);

            if (useState)
                swap(state->get(branchIndex), &context->currentScopeState);

            // Evaluate each term
            for (int j=0; j < acceptedBranch->length(); j++) {
                evaluate_single_term(context, acceptedBranch->get(j));
                if (evaluation_interrupted(context))
                    break;

                acceptedBranchWasEmpty = false;
            }

            if (useState)
                swap(state->get(branchIndex), &context->currentScopeState);

            acceptedBranchIndex = branchIndex;
            break;
        }
    }

    // Reset state for all non-accepted branches
    if (useState) {
        for (int i=0; i < numBranches; i++) {
            if ((i != acceptedBranchIndex) && !acceptedBranchWasEmpty)
                set_null(state->get(i));
        }
        preserve_state_result(CALLER, &prevScopeState, &localState);
        swap(&prevScopeState, &context->currentScopeState);
    }

    // Copy joined values to output slots
    Branch& joining = contents[contents.length()-1]->nestedContents;

    for (int i=0; i < joining.length(); i++) {
        Term* joinTerm = joining[i];
        TaggedValue* value = get_input(joinTerm, acceptedBranchIndex);

        ca_test_assert(cast_possible(value, unbox_type(get_output_type(CALLER, i+1))));

        copy(value, EXTRA_OUTPUT(i));
    }

    // Finish using the branch, this will pop its stack frame. Need to do this after
    // copying joined values.
    finish_using(*acceptedBranch);
}

} // namespace circa
