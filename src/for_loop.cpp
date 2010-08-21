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

#ifndef BYTECODE
List* get_for_loop_state(Term* forTerm)
{
    if (get_for_loop_state_type(forTerm) == VOID_TYPE)
        return NULL;

    Term* term = forTerm->input(0);
    return List::checkCast(term);
}

bool for_loop_has_state(Term* forTerm)
{
    return get_for_loop_state_type(forTerm) != VOID_TYPE;
}

TaggedValue* get_for_loop_iteration_state(Term* forTerm, int index)
{
    return get_for_loop_state(forTerm)->get(index);
}
#endif

Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[1]->nestedContents;
}

Term* get_for_loop_iterator(Term* forTerm)
{
#ifdef BYTECODE
    return forTerm->nestedContents[1];
#else
    return forTerm->nestedContents[2];
#endif
}

#ifndef BYTECODE
Term* get_for_loop_is_first_iteration(Term* forTerm)
{
    return forTerm->nestedContents[0]->nestedContents[0];
}

Term* get_for_loop_any_iterations(Term* forTerm)
{
    return forTerm->nestedContents[0]->nestedContents[1];
}
#endif

Term* get_for_loop_modify_list(Term* forTerm)
{
    #ifdef BYTECODE
    return forTerm->nestedContents[0]->nestedContents[0];
    #else
    return forTerm->nestedContents[0]->nestedContents[2];
    #endif
}

#ifndef BYTECODE
Term* get_for_loop_discard_called(Term* forTerm)
{
    return forTerm->nestedContents[0]->nestedContents[3];
}

Ref& get_for_loop_state_type(Term* forTerm)
{
    return forTerm->nestedContents[0]->nestedContents[4]->asRef();
}
#endif

Branch& get_for_loop_rebinds_for_outer(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[contents.length()-1]->nestedContents;
}

void setup_for_loop_pre_code(Term* forTerm)
{
    Branch& forContents = forTerm->nestedContents;
    Branch& attributes = create_branch(forContents, "#attributes");
#ifdef BYTECODE
    create_bool(attributes, false, "#modify_list");
#else
    create_bool(attributes, false, "#is_first_iteration");
    create_bool(attributes, false, "#any_iterations");
    create_bool(attributes, false, "#modify_list");
    create_bool(attributes, false, "#discard_called");
    create_ref(attributes, VOID_TYPE, "#state_type");
    create_branch(forContents, "#rebinds");
#endif
}

void setup_for_loop_post_code(Term* forTerm)
{
    Branch& forContents = forTerm->nestedContents;
    Branch& outerScope = *forTerm->owningBranch;
#ifdef BYTECODE
    std::string listName = forTerm->input(0)->name;
#else
    std::string listName = forTerm->input(1)->name;
#endif
#ifndef BYTECODE

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

            apply(rebinds, COND_FUNC, RefList(get_for_loop_is_first_iteration(forTerm),
                outerVersion, innerVersion), name);
        }
    }

    // Rewrite code to use these rebound versions
    for (int i=0; i < rebinds.length(); i++) {
        Term* ifexpr = rebinds[i];
        Term* outerVersion = ifexpr->input(1);

        remap_pointers(forContents, outerVersion, ifexpr);

        // undo remap to the arguments of cond()
        ifexpr->inputs[1] = outerVersion;
    }

    // Now do another rebinding, this one has copies that we expose outside of this branch.
    // If the for loop isn't executed at all then we use outer versions, similar to an if()
    // rebinding.
    Branch& rebindsForOuter = create_branch(forContents, "#outer_rebinds");
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

            apply(rebindsForOuter, COND_FUNC,
                    RefList(anyIterations, innerVersion, outerVersion), name);
        }
    }

    // Also, possibly rebind the list name.
    if (as_bool(get_for_loop_modify_list(forTerm)) && listName != "")
        create_value(rebindsForOuter, LIST_TYPE, listName);

    expose_all_names(rebindsForOuter, outerScope);
#endif

#ifdef BYTECODE
    // Create a branch that has all the names which are rebound in this loop
    Branch& outerRebinds = create_branch(forContents, "#outer_rebinds");

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

#endif

#ifndef BYTECODE
    // Figure out if this loop has any state
    bool hasState = false;
    for (int i=0; i < forContents.length(); i++) {
        if (is_stateful(forContents[i])) {
            hasState = true;
            break;
        }
    }

    get_for_loop_state_type(forTerm) = hasState ? LIST_TYPE : VOID_TYPE;
#endif
}

#ifndef BYTECODE
CA_FUNCTION(evaluate_for_loop)
{
    Term* listTerm = INPUT_TERM(1);
    Branch& codeBranch = CALLER->nestedContents;
    List* state = get_for_loop_state(CALLER);

    int numIterations = listTerm->numElements();
    bool modifyList = get_for_loop_modify_list(CALLER)->asBool();
    Term* discardCalled = get_for_loop_discard_called(CALLER);

    Term* listOutput = NULL;
    int listOutputWriteHead = 0;

    if (modifyList)
        listOutput = get_for_loop_rebinds_for_outer(CALLER)[listTerm->name];

    if (state) {
        state->resize(numIterations);
    }

    Term* isFirstIteration = get_for_loop_is_first_iteration(CALLER);
    ca_assert(isFirstIteration->name == "#is_first_iteration");
    Term* iterator = get_for_loop_iterator(CALLER);
    set_bool(get_for_loop_any_iterations(CALLER), numIterations > 0);

    if (numIterations == 0)
        evaluate_branch(CONTEXT, get_for_loop_rebinds_for_outer(CALLER));

    if (state != NULL) {
        //std::cout << "state branch = " << std::endl;
    }

    //std::cout << "Num iterations = " << numIterations << std::endl;

    for (int i=0; i < numIterations; i++) {
        set_bool(isFirstIteration, i == 0);
        set_bool(discardCalled, false);

        // Inject iterator value
        copy((*listTerm)[i], iterator);

        // Inject stateful terms
        if (state != NULL)
            load_state_into_branch(state->get(i), codeBranch);

        //std::cout << "pre evaluate = " << std::endl;
        //dump_branch(codeBranch);

        // Evaluate
        evaluate_branch(CONTEXT, codeBranch);

        //std::cout << "post evaluate = " << std::endl;
        //dump_branch(codeBranch);

        if (CONTEXT->errorOccurred) {
            //std::cout << "Error occurred" << std::endl;
            break;
        }

        // Persist stateful terms
        if (state != NULL) {
            persist_state_from_branch(codeBranch, state->get(i));

            //std::cout << "Persisted = " << std::endl;
        }

        // Possibly use this value to modify the list
        if (listOutput != NULL && !as_bool(discardCalled)) {
            Term* iteratorResult = codeBranch[iterator->name];

            if (listOutputWriteHead >= listOutput->numElements())
                (List::checkCast(listOutput))->append();
            TaggedValue* outputElement = listOutput->getIndex(listOutputWriteHead++);
        
            copy(iteratorResult, outputElement);
        }
    }

    if (listOutput != NULL && listOutput->numElements() > listOutputWriteHead)
        (List::checkCast(listOutput))->resize(listOutputWriteHead);
}
#endif

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

    forTerm->stackIndex = context->nextStackIndex++;
    int outputList = forTerm->stackIndex;
    ca_assert(outputList != -1);
    bool writingOutputList = true;

    bool assignStackIndexes = context->writePos != NULL;
    if (assignStackIndexes) {
        // For names in #outer_rebinds, the join terms should have the same stack
        // indices as the term's output.
        for (int i=0; i < outerRebinds.length(); i++) {
            int stackIndex = context->nextStackIndex++;
            outerRebinds[i]->stackIndex = stackIndex;

            // Don't treat the list name as a name to join, this is handled differently
            if (outerRebinds[i]->name == listName)
                continue;

            forContents[outerRebinds[i]->name]->stackIndex = stackIndex;
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
    int branchOutput = bytecode::write_bytecode_for_branch(context,
            forContents, iterationLocalState,
            2, forContents.length()-1);

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
