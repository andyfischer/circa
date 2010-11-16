// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"

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
    joining.clear();

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

    // Fetch a list container for the state in this block.
    int stateContainer = -1;
    int stateContainerName = -1;
    if (hasState) {

        // State field name.
        TaggedValue stateName;
        set_string(&stateName, get_implicit_state_name(ifBlock));
        stateContainerName = bytecode::write_push_local_op(context, &stateName);

        // State default value
        TaggedValue defaultValue;
        set_list(&defaultValue);
        int stateDefaultValue = bytecode::write_push_local_op(context, &defaultValue);

        // get_state_field
        stateContainer = context->nextRegisterIndex++;
        bytecode::write_get_state_field(context, NULL, stateContainerName, stateDefaultValue,
                stateContainer);

        // push(numBranches)
        numBranchesStack = context->nextRegisterIndex++;
        bytecode::write_push_int(context, numBranches, numBranchesStack);

        // Resize state list
        {
            int inputs[] = { stateContainer, numBranchesStack };
            bytecode::write_call_op(context, NULL, get_global("resize"), 2, inputs,
                stateContainer);
        }
    }

    // For each name in #joining, we want corresponding variables (across the
    // branches in this if-block) to have the same stack indexes.
    for (int i=0; i < joining.length(); i++) {
        ca_assert(joining[i] != NULL);
        if (joining[i]->registerIndex == -1)
            joining[i]->registerIndex = context->nextRegisterIndex++;
        std::string const& name = joining[i]->name;
        ca_assert(name != "");

        // Find the corresponding term in each branch, give it this stack index.
        for (int b=0; b < numBranches; b++) {
            Term* term = blockContents[b]->nestedContents[name];
            if (term != NULL)
                term->registerIndex = joining[i]->registerIndex;
        }
    }
    
    // Go through each branch
    bytecode::BytecodePosition lastBranchJumpOp(NULL,0);
    std::vector<bytecode::BytecodePosition> jumpsToEnd;

    // Stack position for condition-specific state.
    int conditionLocalState = -1;
    int branchIndexStack = -1;
    if (hasState) {
        conditionLocalState = context->nextRegisterIndex++;
        branchIndexStack = context->nextRegisterIndex++;
    }

    for (int branch=0; branch < numBranches; branch++) {
        Term* term = blockContents[branch];

        // Check if we need to write the address for the previous branch's jump
        if (lastBranchJumpOp.data) {
            ((bytecode::JumpIfNotOperation*) lastBranchJumpOp.get())->offset = context->getOffset();
            lastBranchJumpOp = bytecode::BytecodePosition(NULL, 0);
        }

        // Jump check, for 'if' and 'elsif'. Don't know offset yet, will write it later.
        if (term->function == IF_FUNC) {
            lastBranchJumpOp = context->getPosition();
            bytecode::write_jump_if_not(context, term->input(0)->registerIndex, 0);
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

        // For each local rebind, make sure that the branch writes to its stack position.
        // If one branch doesn't mention a local rebind, then insert a copy term.
        for (int i=0; i < joining.length(); i++) {
            std::string const& name = joining[i]->name;
            if (branchContents[name] == NULL) {
                Term* outerVersion = get_named_at(ifBlock, name);
                ca_assert(outerVersion != NULL);
                bytecode::write_copy(context, outerVersion->registerIndex,
                        joining[i]->registerIndex);
            }
        }

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
            jumpsToEnd.push_back(context->getPosition());
            bytecode::write_jump(context, 0);
        }
    }

    for (size_t i=0; i < jumpsToEnd.size(); i++)
        ((bytecode::JumpOperation*) jumpsToEnd[i].get())->offset = context->getOffset();

    // Wrap up state container
    if (hasState) {
        int inputs[] = { context->inlineState, stateContainerName, stateContainer };
        bytecode::write_call_op(context, NULL, get_global("set_state_field"), 3, inputs,
                context->inlineState);
    }
}

CA_FUNCTION(evaluate_if_block)
{
    Branch& contents = CALLER->nestedContents;

    int acceptedBranchIndex = 0;

    for (int i=0; i < contents.length() - 1; i++) {
        Term* branch = contents[i];

        //std::cout << "checking: " << get_term_to_string_extended(branch) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        if (branch->numInputs() == 0 || as_bool(get_input(CONTEXT, branch, 0))) {

            Branch& contents = branch->nestedContents;

            push_stack_frame(STACK, contents.registerCount);
            evaluate_branch_existing_frame(CONTEXT, contents);

            acceptedBranchIndex = i;
            break;
        }
    }

    // Copy the results of our #join terms to the stack
    Branch& joining = contents[contents.length()-1]->nestedContents;
    List* outputFrame = List::checkCast(STACK->get(STACK->length() - 2));

    for (int i=0; i < joining.length(); i++) {
        Term* joinTerm = joining[i];
        TaggedValue* val = get_input(CONTEXT, joinTerm, acceptedBranchIndex);
        copy(val, outputFrame->get(joinTerm->registerIndex));
    }

    pop_stack_frame(STACK);
}

} // namespace circa
