// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
   [1] #inner_rebinds
   [2] iterator
   [...] contents
   [n-1] #outer_rebinds
*/

static const int inner_rebinds_location = 1;
static const int iterator_location = 2;
static const int loop_contents_location = 3;

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->nestedContents[iterator_location];
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
    hide_from_source(result);
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

        Term* original = get_named_at(forTerm, name);

        Term* loopResult = forContents[name];

        // First input to both of these should be 'original', but we need to wait until
        // after remap_pointers before setting this.
        Term* innerRebind = apply(innerRebinds, JOIN_FUNC, RefList(NULL, loopResult), name);
        change_type(innerRebind, original->type);
        Term* outerRebind = apply(outerRebinds, JOIN_FUNC, RefList(NULL, loopResult), name);

        // Rewrite the loop code to use our local copies of these rebound variables.
        remap_pointers(forContents, original, innerRebind);

        set_input(innerRebind, 0, original);
        set_input(outerRebind, 0, original);
    }

    expose_all_names(outerRebinds, outerScope);
    update_register_indices(forContents);
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
            term->registerIndex = context->nextRegisterIndex++;
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
    int iteratorIndex = context->nextRegisterIndex++;

    int listLength = context->nextRegisterIndex++;
    bytecode::write_comment(context, "length(input_list) -> listLength");
    bytecode::write_num_elements(context, inputList, listLength);

    // Fetch state container
    int stateContainer = -1;
    int stateContainerName = -1;

    if (hasState) {
        // State field name.
        TaggedValue stateName;
        set_string(&stateName, get_implicit_state_name(forTerm));
        stateContainerName = bytecode::write_push_local_op(context, &stateName);

        // State default value
        TaggedValue defaultValue;
        set_list(&defaultValue);
        int stateDefaultValue = bytecode::write_push_local_op(context, &defaultValue);

        // get_state_field
        stateContainer = context->nextRegisterIndex++;
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

    int compareIndexOutput = context->nextRegisterIndex++;

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
                term->registerIndex = context->nextRegisterIndex++;
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
        iteratorTerm->registerIndex = context->nextRegisterIndex++;

    // get_index(inputList, iteratorIndex) -> iterator
    bytecode::write_comment(context, "inputList[iteratorIndex] -> iterator");
    bytecode::write_get_index(context, inputList, iteratorIndex,
            iteratorTerm->registerIndex);

    // Fetch state for this iteration
    int iterationLocalState = -1;
    if (hasState) {
        iterationLocalState = context->nextRegisterIndex++;
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

CA_FUNCTION(evaluate_for_loop)
{
    Branch& forContents = CALLER->nestedContents;
    Branch& innerRebinds = forContents[inner_rebinds_location]->nestedContents;
    Branch& outerRebinds = forContents[forContents.length()-1]->nestedContents;
    Term* iterator = get_for_loop_iterator(CALLER);

    TaggedValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    TaggedValue outputTv;
    bool saveOutput = forContents.outputRegister != -1;
    List* output = set_list(&outputTv, inputListLength);
    int nextOutputIndex = 0;

    // Preserve old for-loop context
    ForLoopContext prevLoopContext = CONTEXT->forLoopContext;

    #if 0
    List previousFrame;
    #endif

    for (int iteration=0; iteration < inputListLength; iteration++) {
        bool firstIter = iteration == 0;

        ca_assert(forContents.registerCount > 0);

        #if 0
        List* frame = push_stack_frame(STACK, forContents.registerCount);
        #endif

        // copy iterator
        copy(inputList->getIndex(iteration), get_local(iterator));

        // copy inner rebinds
        for (int i=0; i < innerRebinds.length(); i++) {
            Term* rebindTerm = innerRebinds[i];
            TaggedValue* dest = get_local(rebindTerm);

            if (firstIter)
                copy(get_input(CONTEXT, rebindTerm, 0), dest);
            else
                copy(get_input(CONTEXT, rebindTerm, 1), dest);
        }

        //dump_branch(forContents);

        CONTEXT->forLoopContext.discard = false;

        for (int i=loop_contents_location; i < forContents.length() - 1; i++)
            evaluate_single_term(CONTEXT, forContents[i]);

        // Save output
        if (saveOutput && !CONTEXT->forLoopContext.discard) {
            copy(get_local(forContents[forContents.outputRegister]),
                output->get(nextOutputIndex++));
        }
    }

    // Resize output, in case some elements were discarded
    output->resize(nextOutputIndex);

    swap(output, OUTPUT);

    // Copy outer rebinds
    for (int i=0; i < outerRebinds.length(); i++) {

        Term* rebindTerm = outerRebinds[i];

        TaggedValue* result = NULL;

        if (inputListLength == 0) {
            // No iterations, use the outer rebind
            result = get_input(CONTEXT, rebindTerm, 0);
        } else {
            // At least one iteration, use our local rebind
            result = get_input(CONTEXT, rebindTerm, 1);
        }

        copy(result, get_local(rebindTerm));
    }

    // Restore loop context
    CONTEXT->forLoopContext = prevLoopContext;
}

void for_loop_assign_registers(Term* term)
{
    int next = 0;

    // Iterator goes in register 0
    Term* iterator = get_for_loop_iterator(term);
    next = assign_register(iterator, next);

    // Inner_rebinds go in 1..n
    Branch& forContents = term->nestedContents;
    Branch& innerRebinds = forContents[inner_rebinds_location]->nestedContents;

    for (int i=0; i < innerRebinds.length(); i++)
        next = assign_register(innerRebinds[i], next);

    for (int i=loop_contents_location; i < forContents.length() - 1; i++) {
        if (forContents[i] == NULL) continue;
        next = assign_register(forContents[i], next);
    }

    if (forContents["#outer_rebinds"] != NULL) {
        Branch& outerRebinds = forContents["#outer_rebinds"]->nestedContents;
        for (int i=0; i < outerRebinds.length(); i++) {
            outerRebinds[i]->registerIndex = term->registerIndex + 1 + i;
        }
    }

    forContents.registerCount = next;

    // Figure out the output register. If this is a list-rewrite, then the output
    // is the last term that has the iterator's name binding. Otherwise just use
    // the last term.
    if (as_bool(get_for_loop_modify_list(term))) {
        Term* output = forContents[get_for_loop_iterator(term)->name];
        ca_assert(output != NULL);
        forContents.outputRegister = output->registerIndex;
    } else {
        forContents.outputRegister = forContents.registerCount - 1;
    }
}

} // namespace circa
