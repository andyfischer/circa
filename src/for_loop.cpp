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
    create_bool(attributes, false, "#modify_list");

    Branch& innerRebinds = create_branch(forContents, "#inner_rebinds");
    innerRebinds.owningTerm->setBoolProp("exposesNames", true);
}

Term* setup_for_loop_iterator(Term* forTerm, const char* name)
{
    Term* iteratorType = find_type_of_get_index(forTerm->input(0));
    Term* result = create_value(forTerm->nestedContents, iteratorType, name);
    hide_from_source(result);
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
            TaggedValue* localResult = get_local(forContents[forContents.outputRegister]);
            copy(localResult, output->get(nextOutputIndex++));
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
        //forContents.outputRegister = output->registerIndex;
        forContents.outputRegister = output->index;
    } else {
        forContents.outputRegister = forContents.length() - 1;
    }
}

} // namespace circa
