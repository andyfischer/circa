// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

/*
   Some notes on handling state inside a subroutine:
  
   when each term evaluates, we will look for EvalContext.currentScopeState(), and
   do a lookup using the term's name

Start evaluating a subroutine:
 - Grab state container from currentScopeState
 - Preserve parent currentScopeState
 - Assign this to EvalContext.currentScopeState
 - Evaluate each term
 - Assign modified state back into the parent currentScopeState
 - Restore parent currentScopeState

*/
 
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

    void evaluate_subroutine(EvalContext* context, Term* caller)
    {
        Term* function = caller->function;
        Branch& contents = function->nestedContents;
        context->interruptSubroutine = false;

        // Copy inputs to a temporary list
        List inputs;
        inputs.resize(caller->numInputs());

        for (int i=0; i < caller->numInputs(); i++) {
            TaggedValue* input = get_input(context, caller, i);
            if (input == NULL)
                continue;

            Type* inputType = type_contents(function_t::get_input_type(function, i));

            bool success = cast(input, inputType, inputs[i]);
            ca_assert(success);
        }

        start_using(contents);

        // Insert inputs into placeholdes
        for (int i=0; i < caller->numInputs(); i++) {
            Term* placeholder = function_t::get_input_placeholder(function, i);
            swap(inputs[i], get_local(placeholder));
        }

        // prepare output
        set_null(&context->subroutineOutput);

        // Fetch state container
        TaggedValue prevScopeState;
        swap(&context->currentScopeState, &prevScopeState);

        if (is_function_stateful(function))
            fetch_state_container(caller, &prevScopeState, &context->currentScopeState);

        // Evaluate each term
        for (int i=0; i < contents.length(); i++) {
            evaluate_single_term(context, contents[i]);
            if (context->interruptSubroutine)
                break;
        }

        //std::cout << "finished subroutine, stack = " << context->stack.toString() <<std::endl;
        //dump_branch(contents);

        // Fetch output
        Term* outputTypeTerm = function_t::get_output_type(caller->function);
        Type* outputType = type_contents(outputTypeTerm);
        TaggedValue output;

        if (outputTypeTerm != VOID_TYPE) {
            TaggedValue* outputSource = NULL;

            if (!is_null(&context->subroutineOutput)) {
                outputSource = &context->subroutineOutput;
            }
            else {
                outputSource = get_local(contents[contents.length()-1]);
            }

            //std::cout << "found output: " << outputSource->toString() << std::endl;

            bool success = cast(outputSource, outputType, &output);
            
            if (!success) {
                std::stringstream msg;
                msg << "Couldn't cast output " << output.toString()
                    << " to type " << outputType->name;
                error_occurred(context, caller, msg.str());
            }

            set_null(&context->subroutineOutput);
        }

        // Write to state
        wrap_up_open_state_vars(context, contents);

        // Restore currentScopeState
        if (is_function_stateful(function))
            preserve_state_result(caller, &prevScopeState, &context->currentScopeState);

        swap(&context->currentScopeState, &prevScopeState);

        finish_using(contents);

        TaggedValue* outputDest = get_output(context, caller);
        if (outputDest != NULL)
            swap(&output, outputDest);
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
    return function_t::get_evaluate(term) == subroutine_t::evaluate_subroutine;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    // Install evaluate function
    function_t::get_evaluate(sub) = subroutine_t::evaluate_subroutine;

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

    bool hasState = has_implicit_state(func);
    #if 0
    Branch& contents = func->nestedContents;
    for (int i=0; i < contents.length(); i++) {
        Term* term = contents[i];
        if (term == NULL)
            continue;
        if (is_get_state(term)) {
            hasState = true;
            break;
        }
        if (is_subroutine(term->function)) {
            if (is_function_stateful(term->function)) {
                hasState = true;
                break;
            }
        }
        if (term->function == IF_BLOCK_FUNC) {
            if (if_block

        }
    }
    #endif

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
        Term* input = contents[1];
        rewrite(input, INPUT_PLACEHOLDER_FUNC, RefList());
        contents.bindName(input, "#state");
        input->setBoolProp("optional", true);
    }

    // If state was added, find all the recursive calls to this function and
    // insert a state argument.
    if (previousType == VOID_TYPE && newType != VOID_TYPE) {
        for (BranchIterator it(contents); !it.finished(); ++it) {
            Term* term = *it;

            if (term->function == func) {
                Branch* branch = term->owningBranch;
                Term* stateType = function_t::get_inline_state_type(func);
                Term* stateContainer = create_stateful_value(*branch, stateType, NULL,
                        term->uniqueName.name);
                branch->move(stateContainer, term->index);

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
    set_list(storageTv);
    List* storage = List::checkCast(storageTv);
    storage->resize(branch.length());
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == FUNCTION_ATTRS_TYPE)
            continue;

        copy(term, storage->get(i));
    }
}

void restore_locals(TaggedValue* storageTv, Branch& branch)
{
    if (!is_list(storageTv))
        internal_error("storageTv is not a list");

    List* storage = List::checkCast(storageTv);

    // The function branch may be longer than our list of locals. 
    int numItems = storage->length();
    for (int i=0; i < numItems; i++) {
        Term* term = branch[i];

        if (term == NULL) continue;

        if (term->type == FUNCTION_ATTRS_TYPE)
            continue;

        copy(storage->get(i), term);
    }
}

} // namespace circa
