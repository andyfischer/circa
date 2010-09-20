// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
   [1] iterator
   [2] #inner_rebinds
   [...] contents
   [n-1] #outer_rebinds
*/

Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[1]->nestedContents;
}

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->nestedContents[2];
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

    Branch& innerRebinds = create_branch(forContents, "#inner_rebinds");
    innerRebinds.owningTerm->setBoolProp("no-bytecode", true);
    innerRebinds.owningTerm->setBoolProp("exposesNames", true);
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
    std::string iteratorName = get_for_loop_iterator(forTerm)->name;

    // Create a branch that has all the names which are rebound in this loop
    Branch& innerRebinds = forContents["#inner_rebinds"]->nestedContents;
    Branch& outerRebinds = create_branch(forContents, "#outer_rebinds");
    outerRebinds.owningTerm->setBoolProp("no-bytecode", true);

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(forContents, reboundNames);

    for (size_t i=0; i < reboundNames.size(); i++) {
        std::string const& name = reboundNames[i];
        if (name == listName)
            continue;
        if (name == iteratorName)
            continue;

        Term* original = outerScope[name];

        Term* innerRebind = apply(innerRebinds, JOIN_FUNC, RefList(), name);
        change_type(innerRebind, original->type);
        apply(outerRebinds, JOIN_FUNC, RefList(), name);

        // Rewrite the loop code to use our local copies of these rebound variables.
        remap_pointers(forContents, original, innerRebind);
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
    Branch& innerRebinds = forContents["#inner_rebinds"]->nestedContents;
    Branch& outerRebinds = forContents["#outer_rebinds"]->nestedContents;
    Branch& outerScope = *forTerm->owningBranch;
    bool modifyList = as_bool(get_for_loop_modify_list(forTerm));
    Term* inputTerm = forTerm->input(0);
    std::string const& listName = inputTerm->name;
    bool hasState = has_any_inlined_state(forContents);
    bytecode::assign_register_index(context, forTerm);
    int outputList = forTerm->registerIndex;
    ca_assert(outputList != -1);
    bool writingOutputList = true;

    // Assign stack indices
    for (int i=0; i < innerRebinds.length(); i++) {
        Term* term = innerRebinds[i];
        if (term->registerIndex == -1)
            term->registerIndex = context->nextStackIndex++;
    }

    // For names in #outer_rebinds, the join terms should have the same stack
    // indices as the term's output.
    for (int i=0; i < outerRebinds.length(); i++) {
        Term* outerRebind = outerRebinds[i];
        bytecode::assign_register_index(context, outerRebind);

        // Don't treat the list name as a name to join, this is handled differently
        if (outerRebind->name == listName)
            continue;

        forContents[outerRebinds[i]->name]->registerIndex = outerRebind->registerIndex;
    }

    int inputList = inputTerm->registerIndex;
    int iteratorIndex = context->nextStackIndex++;

    int listLength = context->nextStackIndex++;
    bytecode::write_comment(context, "length(input_list) -> listLength");
    bytecode::write_num_elements(context, inputList, listLength);

    // Fetch state container
    int stateContainer = -1;
    int stateContainerName = -1;

    if (hasState) {
        // State field name.
        TaggedValue stateName;
        make_string(&stateName, get_implicit_state_name(forTerm));
        stateContainerName = bytecode::write_push_local_op(context, &stateName);

        // State default value
        TaggedValue defaultValue;
        make_list(&defaultValue);
        int stateDefaultValue = bytecode::write_push_local_op(context, &defaultValue);

        // get_state_field
        stateContainer = context->nextStackIndex++;
        bytecode::write_get_state_field(context, NULL, stateContainerName, stateDefaultValue,
                stateContainer);

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

    // If we're outputing a list, then initialize a blank list output here.
    if (writingOutputList) {
        int inputs[1];
        inputs[0] = listLength;
        bytecode::write_comment(context, "blank_list -> outputList");
        bytecode::write_call_op(context, NULL,
                get_global("blank_list"), 1, inputs, outputList);
    }

    // Copy values for any rebinds
    if (innerRebinds.length() > 0) {
        bytecode::write_comment(context, "Copy local rebinds");
        for (int i=0; i < innerRebinds.length(); i++) {
            Term* term = innerRebinds[i];
            if (term->registerIndex == -1)
                term->registerIndex = context->nextStackIndex++;
            Term* outerVersion = get_named_at(outerScope, forTerm->index, outerRebinds[i]->name);
            bytecode::write_copy(context, outerVersion->registerIndex, term->registerIndex);
        }
    }

    // Do another copy for rebind output, in case the loop has 0 iterations. This could be 
    // optimized but we'll optimize later.
    bytecode::write_comment(context, "Copy outer rebinds, in case loop has 0 iterations");
    for (int i=0; i < outerRebinds.length(); i++) {
        Term* rebind = outerRebinds[i];
        Term* outerVersion = get_named_at(outerScope, forTerm->index, outerRebinds[i]->name);
        bytecode::write_copy(context, outerVersion->registerIndex, rebind->registerIndex);
    }

    // loop_start: Check if index < length(input_list)
    int loopStartPos = context->getOffset();

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
    bytecode::BytecodePosition jumpToEnd = context->getPosition();
    ca_assert(compareIndexOutput != -1);
    bytecode::write_jump_if_not(context, compareIndexOutput, 0);

    Term* iteratorTerm = get_for_loop_iterator(forTerm);
    if (iteratorTerm->registerIndex == -1)
        iteratorTerm->registerIndex = context->nextStackIndex++;

    // get_index(inputList, iteratorIndex) -> iterator
    bytecode::write_comment(context, "inputList[iteratorIndex] -> iterator");
    bytecode::write_get_index(context, inputList, iteratorIndex,
            iteratorTerm->registerIndex);

    // Copy local rebinds to their output slots
    /*for (int i=0; i < outerRebinds.length(); i++) {
        Term* outerVersion = get_named_at(outerScope, forTerm->index, outerRebinds[i]->name);
        ca_assert(outerVersion != NULL);
        bytecode::write_copy(context, outerVersion->registerIndex, outerRebinds[i]->registerIndex);
    }*/

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
            result = modifiedIterator->registerIndex;
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

    bytecode::write_comment(context, "Copy locals back to innerRebinds");
    for (int i=0; i < innerRebinds.length(); i++) {
        Term* original = innerRebinds[i];
        Term* modified = outerRebinds[i];
        bytecode::write_copy(context, modified->registerIndex, original->registerIndex);
    }

    // increment(iterator_index)
    bytecode::write_comment(context, "increment iterator");
    bytecode::write_increment(context, iteratorIndex);

    // jump back to loop_start
    bytecode::write_comment(context, "jump back to loop start");
    bytecode::write_jump(context, loopStartPos);

    // complete the above jumpToEnd
    ((bytecode::JumpIfNotOperation*) jumpToEnd.get())->offset = context->getOffset();

    // Wrap up state container
    if (hasState) {
        int inputs[] = { context->inlineState, stateContainerName, stateContainer };
        bytecode::write_call_op(context, NULL, get_global("set_state_field"), 3, inputs,
                context->inlineState);
    }
}

} // namespace circa
