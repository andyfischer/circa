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
//
// Bytecode:
//
// if <expr0>
//   ..branch A..
//   name = x
// elif <expr1>
//   ..branch B..
//   name = y
// else
//   ..branch Z..
// end
// name = join()
//
// 0: expr0
// 1: jump_if_not(0) to 2
// .. branch A
// .. jump to End
// 2: expr1
//    jump_if_not(2) to
// .. branch B
// ..
// End
//
// Algorithm:
//   Figure out stack positions for exported names

void update_if_block_joining_branch(Term* ifCall)
{
    Branch& contents = ifCall->nestedContents;

    // Create the joining contents if necessary
    if (!contents.contains("#joining"))
        create_branch(contents, "#joining");

    Branch& joining = contents["#joining"]->nestedContents;
    joining.clear();


    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    {
        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Term* term = contents[branch_index];
            Branch& branch = is_branch(term) ? as_branch(term) : term->nestedContents;

            TermNamespace::const_iterator it;
            for (it = branch.names.begin(); it != branch.names.end(); ++it)
                boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    ca_assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // Ignore hidden names
        if (name[0] == '#') {
            boundNames.erase(it++);
            continue;
        }

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_named(*outerScope, name) != NULL;

        int numberOfBranchesWithThisName = 0;
        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Term* term = contents[branch_index];
            Branch& branch = is_branch(term) ? as_branch(term) : term->nestedContents;
            if (branch.contains(name))
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

        apply(joining, JOIN_FUNC, RefList(), name);
    }

    // Expose all names in 'joining' branch.
    expose_all_names(joining, *outerScope);
}

Branch* get_if_condition_block(Term* ifCall, int index)
{
    ca_assert(ifCall->function = IF_BLOCK_FUNC);
    Branch& callContents = ifCall->nestedContents;
    ca_assert(index < callContents.length());
    return &(callContents[index]->nestedContents);
}

Branch* get_if_block_else_block(Term* ifCall)
{
    ca_assert(ifCall->function = IF_BLOCK_FUNC);
    Branch& callContents = ifCall->nestedContents;
    ca_assert(callContents.length() >= 2);
    return &(callContents[callContents.length()-2]->nestedContents);
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
    Branch& contents = ifCall->nestedContents;
    for (int cond=0; cond < contents.length(); cond++) {
        Branch& condContents = contents[cond]->nestedContents;
        if (has_any_inlined_state(condContents))
            return true;
    }
    return false;
}


void write_if_block_bytecode(bytecode::WriteContext* context, Term* ifBlock)
{
    Branch& blockContents = ifBlock->nestedContents;
    Branch& joining = blockContents["#joining"]->nestedContents;
    bool hasState = if_block_contains_state(ifBlock);

    int numBranches = blockContents.length() - 1;
    int numBranchesStack = -1;
    bool assignStackIndexes = context->writePos != NULL;

    // Fetch a list container for the state in this block.
    int stateContainer = -1;
    if (hasState) {
        Term* getState = ifBlock->owningBranch->get(ifBlock->index-1);
        ca_assert(getState != NULL);
        ca_assert(getState->function->name == "get_state_field");
        numBranchesStack = context->nextStackIndex++;
        bytecode::write_push_int(context, numBranches, numBranchesStack);
        stateContainer = getState->stackIndex;

        // Resize state list
        {
            int inputs[] = { stateContainer, numBranchesStack };
            bytecode::write_call_op(context, NULL, get_global("resize"), 2, inputs,
                stateContainer);
        }
    }

    if (assignStackIndexes) {
        // For each name in #joining, we want corresponding variables (across the
        // branches in this if-block) to have the same stack indexes.
        for (int i=0; i < joining.length(); i++) {
            if (joining[i]->stackIndex == -1)
                joining[i]->stackIndex = context->nextStackIndex++;
            std::string& name = joining[i]->name;
            ca_assert(name != "");

            // Find the corresponding term in each branch, give it this stack index.
            for (int b=0; b < numBranches; b++) {
                Term* term = blockContents[b]->nestedContents[name];
                term->stackIndex = joining[i]->stackIndex;
            }
        }
    }
    
    // Go through each branch
    bytecode::JumpIfNotOperation *lastBranchJumpOp = NULL;
    std::vector<bytecode::JumpOperation*> jumpsToEnd;

    // Stack position for condition-specific state.
    int conditionLocalState = -1;
    int branchIndexStack = -1;
    if (hasState) {
        conditionLocalState = context->nextStackIndex++;
        branchIndexStack = context->nextStackIndex++;
    }

    for (int branch=0; branch < numBranches; branch++) {
        Term* term = blockContents[branch];

        // Check if we need to write the address for the previous branch's jump
        if (lastBranchJumpOp) {
            lastBranchJumpOp->offset = context->getOffset();
            lastBranchJumpOp = NULL;
        }

        // Jump check, for 'if' and 'elsif'. Don't know offset yet, will write it later.
        if (term->function == IF_FUNC) {
            lastBranchJumpOp = (bytecode::JumpIfNotOperation*) context->writePos;
            bytecode::write_jump_if_not(context, term->input(0)->stackIndex, 0);
        }

        // If there's state, then pull out the state for this branch.
        if (hasState) {
            bytecode::write_push_int(context, branch, branchIndexStack);
            bytecode::write_get_index(context, stateContainer,
                    branchIndexStack, conditionLocalState);
        }

        // Write each instruction in this branch
        Branch& branchContents = term->nestedContents;
        bytecode::write_bytecode_for_branch(context, branchContents, conditionLocalState);

        if (hasState) {
            // For all the branches which did not get evaluated, reset state. An
            // easy way to do this is to just reset state to an empty list.
            {
                int inputs[] = { numBranchesStack };
                bytecode::write_call_op(context, NULL, get_global("blank_list"),
                        1, inputs, stateContainer);
            }

            // Save condition-local state into list-wide container
            {
                int inputs[] = { stateContainer, branchIndexStack, conditionLocalState };
                bytecode::write_call_op(context, NULL, get_global("set_index"), 3, inputs,
                    stateContainer);
            }
        }
        
        // Jump past remaining branches. But, don't need to do this for last branch.
        if (branch+1 < numBranches) {
            if (context->writePos)
                jumpsToEnd.push_back((bytecode::JumpOperation*) context->writePos);
            bytecode::write_jump(context, 0);
        }
    }

    if (context->writePos) {
        for (size_t i=0; i < jumpsToEnd.size(); i++)
            jumpsToEnd[i]->offset = context->getOffset();
    }
}

} // namespace circa
