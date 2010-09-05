// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "evaluation.h"
#include "circa.h"
#include "importing_macros.h"

namespace circa {

namespace subroutine_t {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, token::DEF);

        function_t::format_header_source(source, term);

        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                term, token::WHITESPACE);

        if (!is_native_function(term))
            format_branch_source(source, term->nestedContents, term);
    }

    CA_FUNCTION(evaluate)
    {
        // Copy inputs to a new stack
        List stack;
        stack.resize(NUM_INPUTS);
        for (int i=0; i < NUM_INPUTS; i++) {
            TaggedValue* input = INPUT(i);
            if (input != NULL)
                copy(input, stack.get(i));
        }

        reset(&CONTEXT->subroutineOutput);

        Term* function = FUNCTION;
        Branch& functionBranch = function->nestedContents;
        if (!functionBranch._bytecode.inuse)
            bytecode::update_bytecode(functionBranch);
        evaluate_bytecode(CONTEXT, &functionBranch._bytecode, &stack);

        if (OUTPUT != NULL) {
            swap(&CONTEXT->subroutineOutput, OUTPUT);
        }
        reset(&CONTEXT->subroutineOutput);

    }
}

bool is_subroutine(Term* term)
{
    if (term->type != FUNCTION_TYPE)
        return false;
    if (term->nestedContents.length() < 1)
        return false;
    if (term->nestedContents[0]->type != FUNCTION_ATTRS_TYPE)
        return false;
    return function_t::get_evaluate(term) == subroutine_t::evaluate;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    // Install evaluate function
    function_t::get_evaluate(sub) = subroutine_t::evaluate;

    subroutine_update_state_type_from_contents(sub);
}

void subroutine_update_state_type_from_contents(Term* func)
{
    // Check if a stateful argument was declared
    Term* firstInput = function_t::get_input_placeholder(func, 0);
    if (firstInput != NULL && firstInput->boolPropOptional("state", false)) {
        // already updated state
        return;
    }

    bool hasState = false;
    Branch& contents = func->nestedContents;
    for (int i=0; i < contents.length(); i++) {
        if (contents[i] == NULL)
            continue;
        if (is_stateful(contents[i])) {
            hasState = true;
            break;
        }
        if (is_subroutine(contents[i]->function)) {
            if (is_function_stateful(contents[i]->function)) {
                hasState = true;
                break;
            }
        }
    }

    if (hasState)
        subroutine_change_state_type(func, LIST_TYPE);
}

void subroutine_change_state_type(Term* func, Term* newType)
{
    Term* previousType = function_t::get_inline_state_type(func);
    if (previousType == newType)
        return;

    Branch& contents = func->nestedContents;
    function_t::get_attrs(func).implicitStateType = newType;

    bool hasStateInput = (function_t::num_inputs(func) > 0)
        && (function_t::get_input_name(func, 0) == "#state");

    // create a stateful input if needed
    if (newType != VOID_TYPE && !hasStateInput) {
        contents.insert(1, alloc_term());
        rewrite(contents[1], INPUT_PLACEHOLDER_FUNC, RefList());
        contents.bindName(contents[1], "#state");
    }

    // If state was added, find all the recursive calls to this function and
    // insert a state argument.
    if (previousType == VOID_TYPE && newType != VOID_TYPE) {
        for (BranchIterator it(contents); !it.finished(); ++it) {
            Term* term = *it;

            if (term->function == func) {
                Branch* branch = term->owningBranch;
                Term* stateContainer = alloc_term();
                branch->insert(term->index, stateContainer);
                change_type(stateContainer, function_t::get_inline_state_type(func));
                //FIXME
                //change_function(stateContainer, STATEFUL_VALUE_FUNC);
                branch->bindName(stateContainer, default_name_for_hidden_state(term->name));

                RefList inputs = term->inputs;
                inputs.prepend(stateContainer);
                set_inputs(term, inputs);

                // We just inserted a term, so the next iteration will hit the
                // term we just checked. So, advance past this.
                ++it;
            }
        }
    }
}

void store_locals(Branch& branch, TaggedValue* storageTv)
{
    touch(storageTv);
    make_list(storageTv);
    List* storage = List::checkCast(storageTv);
    storage->resize(branch.length());
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == FUNCTION_ATTRS_TYPE)
            continue;

        if (is_branch(term))
            store_locals(term->nestedContents, storage->get(i));
        else
            copy(term, storage->get(i));
    }
}

void restore_locals(TaggedValue* storageTv, Branch& branch)
{
    if (!list_t::is_list(storageTv))
        internal_error("storageTv is not a list");

    List* storage = List::checkCast(storageTv);

    // The function branch may be longer than our list of locals. 
    int numItems = storage->length();
    for (int i=0; i < numItems; i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == FUNCTION_ATTRS_TYPE)
            continue;

        if (is_branch(term))
            restore_locals(storage->get(i), term->nestedContents);
        else
            copy(storage->get(i), term);
    }
}

} // namespace circa
