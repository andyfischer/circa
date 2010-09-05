// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
   [1] iterator
   [2 .. n-2] user's code
   [n-1] #outer_rebinds
*/


Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[1]->nestedContents;
}

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->nestedContents[1];
}


Term* get_for_loop_modify_list(Term* forTerm)
{
    Term* term = forTerm->nestedContents[0]->nestedContents[0];
    ca_assert(term != NULL);
    return term;
}


Branch& get_for_loop_rebinds_for_outer(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[contents.length()-1]->nestedContents;
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch& forContents = forTerm->nestedContents;
    Branch& attributes = create_branch(forContents, "#attributes");
    attributes.owningTerm->setBoolProp("no-bytecode", true);
    create_bool(attributes, false, "#modify_list");
}

Term* setup_for_loop_iterator(Term* forTerm, const char* name)
{
    Term* iteratorType = find_type_of_get_index(forTerm->input(0));
    Term* result = create_value(forTerm->nestedContents, iteratorType, name);
    set_source_hidden(result, true);
    result->setBoolProp("no-bytecode", true);
    return result;
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch& forContents = forTerm->nestedContents;
    Branch& outerScope = *forTerm->owningBranch;
    std::string listName = forTerm->input(0)->name;

    // Create a branch that has all the names which are rebound in this loop
    Branch& outerRebinds = create_branch(forContents, "#outer_rebinds");
    outerRebinds.owningTerm->setBoolProp("no-bytecode", true);

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(forContents, reboundNames);

    for (size_t i=0; i < reboundNames.size(); i++) {
        if (reboundNames[i] == listName)
            continue;

        apply(outerRebinds, JOIN_FUNC, RefList(), reboundNames[i]);
    }

    bool modifyList = as_bool(get_for_loop_modify_list(forTerm));

    if (modifyList)
        apply(outerRebinds, JOIN_FUNC, RefList(), listName);

    expose_all_names(outerRebinds, outerScope);


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

void write_for_loop_bytecode(bytecode::WriteContext* context, Term* forTerm)
{
    // -- Simple version --
    // push 0 -> index
    // loop_start:
    // get_index input_list index -> iterator
    // .. loop contents ..
    // inc index
    // index < input_list->numElements
    // jump_if to: loop_start
    //
    // -- Modify list --
    // push 0 -> index
    //>> push [] -> output_list
    // loop_start:
    // get_index input_list index -> iterator
    // .. loop contents ..
    //>> (for a discard statement, jump to end_of_loop)
    //>> dont_discard:
    //>> append modified_iterator output_list
    // end_of_loop:
    // inc index
    // index < input_list->numElements
    // jump_if to: loop_start
    
    // Assign stack positions to #rebinds first (similar to inside if block)
    Branch& forContents = forTerm->nestedContents;
    Branch& outerRebinds = forContents[forContents.length()-1]->nestedContents;
    Branch& outerScope = *forTerm->owningBranch;
    bool modifyList = as_bool(get_for_loop_modify_list(forTerm));
    Term* inputTerm = forTerm->input(0);
    std::string const& listName = inputTerm->name;
    bool hasState = has_any_inlined_state(forContents);
    bytecode::assign_stack_index(context, forTerm);
    int outputList = forTerm->stackIndex;
    ca_assert(outputList != -1);
    bool writingOutputList = true;

    bool assignStackIndexes = context->writePos != NULL;
    if (assignStackIndexes) {
        // For names in #outer_rebinds, the join terms should have the same stack
        // indices as the term's output.
        for (int i=0; i < outerRebinds.length(); i++) {
            Term* outerRebind = outerRebinds[i];
            bytecode::assign_stack_index(context, outerRebind);

            // Don't treat the list name as a name to join, this is handled differently
            if (outerRebind->name == listName)
                continue;

            forContents[outerRebinds[i]->name]->stackIndex = outerRebind->stackIndex;
        }
    }

    int inputList = inputTerm->stackIndex;
    int iteratorIndex = context->nextStackIndex++;

    int listLength = context->nextStackIndex++;
    bytecode::write_comment(context, "length(input_list) -> listLength");
    bytecode::write_num_elements(context, inputList, listLength);

    // Fetch state container
    int stateContainer = -1;
    if (hasState) {
        Term* getState = forTerm->owningBranch->get(forTerm->index-1);
        ca_assert(getState != NULL);
        ca_assert(getState->function->name == "get_state_field");
        stateContainer = getState->stackIndex;

        // Resize state list
        {
            int inputs[]  = { stateContainer, listLength };
            bytecode::write_comment(context, "resize state list");
            bytecode::write_call_op(context, NULL, get_global("resize"), 2, inputs,
                    stateContainer);
        }
    }
    
    bytecode::write_comment(context, "push 0 -> iteratorIndex");
    bytecode::write_push_int(context, 0, iteratorIndex);

    int compareIndexOutput = context->nextStackIndex++;

    // Do an initial check of iterator vs length, if they fail this check then
    // the loop is never run, and we'll need to copy values for outer rebinds.

    // less_than_i(iterator, listLength) -> compareIndexOutput
    {
        int inputs[2];
        inputs[0] = iteratorIndex;
        inputs[1] = listLength;
        bytecode::write_comment(context, "iterator < listLength -> compareIndexOutput");
        bytecode::write_call_op(context, NULL,
                get_global("less_than_i"), 2, inputs, compareIndexOutput);
    }

    // If we're outputing a list, then initialize a blank list output here.
    if (writingOutputList) {
        int inputs[1];
        inputs[0] = listLength;
        bytecode::write_comment(context, "blank_list -> outputList");
        bytecode::write_call_op(context, NULL,
                get_global("blank_list"), 1, inputs, outputList);
    }

    bytecode::write_comment(context, "jump when there are zero iterations");
    bytecode::JumpIfNotOperation* jumpToLoopNeverRun = (bytecode::JumpIfNotOperation*)
        context->writePos;
    bytecode::write_jump_if_not(context, compareIndexOutput, 0);

    // loop_start: Check if index < length(input_list)
    int loopStartPos = context->getOffset();

    // (this code is duplicated above)
    // less_than_i(iterator, listLength) -> compareIndexOutput
    {
        int inputs[2];
        inputs[0] = iteratorIndex;
        inputs[1] = listLength;
        bytecode::write_comment(context, "iterator < listLength -> compareIndexOutput");
        bytecode::write_call_op(context, NULL,
                get_global("less_than_i"), 2, inputs, compareIndexOutput);
    }

    // jump_if_not(compareIndexOutput) offset:end
    bytecode::write_comment(context, "jump to end if iterator >= listLength");
    bytecode::JumpIfNotOperation *jumpToEnd = (bytecode::JumpIfNotOperation*) context->writePos;
    bytecode::write_jump_if_not(context, compareIndexOutput, 0);

    Term* iteratorTerm = get_for_loop_iterator(forTerm);
    if (iteratorTerm->stackIndex == -1)
        iteratorTerm->stackIndex = context->nextStackIndex++;

    // get_index(inputList, iteratorIndex) -> iterator
    bytecode::write_comment(context, "inputList[iteratorIndex] -> iterator");
    bytecode::write_get_index(context, inputList, iteratorIndex,
            iteratorTerm->stackIndex);

    // Copy local rebinds to their output slots


    // Fetch state for this iteration
    int iterationLocalState = -1;
    if (hasState) {
        iterationLocalState = context->nextStackIndex++;
        bytecode::write_comment(context, "fetch iteration-local state");
        bytecode::write_get_index(context, stateContainer, iteratorIndex,
                iterationLocalState);
    }

    // loop contents
    bytecode::write_comment(context, "loop body:");
    //forContents[forContents.length()-1]->setBoolProp("no-bytecode", true);
    int branchOutput = bytecode::write_bytecode_for_branch(context,
            forContents, iterationLocalState);

    // Save iteration-local state
    if (hasState) {
        bytecode::write_comment(context, "save iteration-local state");
        int inputs[] = { stateContainer, iteratorIndex, iterationLocalState };
        bytecode::write_call_op(context, NULL, get_global("set_index"), 3, inputs,
                stateContainer);
    }

    // Save the list output. If we're rewriting the input list, then the output
    // comes from whatever is bound to the iterator name. Otherwise, the output
    // comes from the last expression inside forContents.
    if (writingOutputList) {
        int result = -1;
        if (modifyList) {
            Term* modifiedIterator = forContents[iteratorTerm->name];
            result = modifiedIterator->stackIndex;
        } else {
            result = branchOutput;
        }

        if (result != -1) {
            int inputs[] = { outputList, iteratorIndex, result };
            bytecode::write_comment(context, "save list output");
            bytecode::write_call_op(context, NULL, get_global("set_index"), 3,
                inputs, outputList);
        }
    }

    // increment(iterator_index)
    bytecode::write_comment(context, "increment iterator");
    bytecode::write_increment(context, iteratorIndex);

    // jump back to loop_start
    bytecode::write_comment(context, "jump back to loop start");
    bytecode::write_jump(context, loopStartPos);

    // Here we insert a block of code that is called when there are 0 iterations.
    if (jumpToLoopNeverRun)
        jumpToLoopNeverRun->offset = context->getOffset();

    for (int i=0; i < outerRebinds.length(); i++) {
        Term* outerVersion = get_named_at(outerScope, forTerm->index, outerRebinds[i]->name);
        ca_assert(outerVersion != NULL);
        bytecode::write_copy(context, outerVersion->stackIndex, outerRebinds[i]->stackIndex);
    }

    // complete the above jumpToEnd
    if (jumpToEnd)
        jumpToEnd->offset = context->getOffset();
}

} // namespace circa
