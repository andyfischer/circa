// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {

/* Organization of for loop contents:
   [0] #attributes
     [0] #modify_list
     [1] #state_type
   [1] iterator
   [2 .. n-2] user's code
   [n-1] #rebinds_for_outer
*/

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

Branch& get_for_loop_rebinds(Term* forTerm)
{
    Branch& contents = forTerm->nestedContents;
    return contents[1]->nestedContents;
}

Term* get_for_loop_iterator(Term* forTerm)
{
    return forTerm->nestedContents[2];
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

Term* get_for_loop_discard_called(Term* forTerm)
{
    return forTerm->nestedContents[0]->nestedContents[3];
}

Ref& get_for_loop_state_type(Term* forTerm)
{
#ifdef BYTECODE
    return forTerm->nestedContents[0]->nestedContents[1]->asRef();
#else
    return forTerm->nestedContents[0]->nestedContents[4]->asRef();
#endif
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
#ifdef BYTECODE
    create_bool(attributes, false, "#modify_list");
    create_ref(attributes, VOID_TYPE, "#state_type");
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
#ifndef BYTECODE
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

            apply(rebindsForOuter, COND_FUNC,
                    RefList(anyIterations, innerVersion, outerVersion), name);
        }
    }

    // Also, possibly rebind the list name.
    if (as_bool(get_for_loop_modify_list(forTerm)) && listName != "")
        create_value(rebindsForOuter, LIST_TYPE, listName);

    expose_all_names(rebindsForOuter, outerScope);
#endif

    // Figure out if this loop has any state
    bool hasState = false;
    for (int i=0; i < forContents.length(); i++) {
        if (is_stateful(forContents[i])) {
            hasState = true;
            break;
        }
    }

    get_for_loop_state_type(forTerm) = hasState ? LIST_TYPE : VOID_TYPE;
}

CA_FUNCTION(evaluate_for_loop)
{
#ifndef BYTECODE
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
#endif
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
    // push [] -> output_list
    // loop_start:
    // get_index input_list index -> iterator
    // .. loop contents ..
    // (for a discard statement, jump to end_of_loop)
    // dont_discard:
    // append modified_iterator output_list
    // end_of_loop:
    // inc index
    // index < input_list->numElements
    // jump_if to: loop_start
    //
    // Assign stack positions to #rebinds first (similar to inside if block)


    int iteratorIndex = context->nextStackIndex++;
    
    // push 0 -> iterator_index
    bytecode::write_push_int(context, 0, iteratorIndex);

    char* loopStartPos = context->writePos;

    // get_index(input_list iterator_index) -> iterator
    Term* iteratorTerm = get_for_loop_iterator(forTerm);
    if (iteratorTerm->stackIndex == -1)
        iteratorTerm->stackIndex = context->nextStackIndex++;

    bytecode::write_get_index(context, forTerm->input(0)->stackIndex, iteratorIndex,
            iteratorTerm->stackIndex);

    // loop contents
    Branch& forContents = forTerm->nestedContents;
    for (int i=1; i < forContents.length(); i++) {
        Term* term = forContents[i];
        bytecode::write_op(context, term);
    }

    // inc iterator_index
    bytecode::write_increment(context, iteratorIndex);

    // index < input_list->numElements
    int numElementsOutput = context->nextStackIndex++;

}

} // namespace circa
