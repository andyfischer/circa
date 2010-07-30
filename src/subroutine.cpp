// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
        Term* function = FUNCTION;
        Branch& functionBranch = function->nestedContents;

        bool varArgs = function_t::get_variable_args(function);
        int num_inputs = function_t::num_inputs(function);

        if (!varArgs && (num_inputs != NUM_INPUTS)) {
            std::stringstream msg;
            msg << "Wrong number of inputs, expected: " << num_inputs
                << ", found: " << NUM_INPUTS;
            error_occurred(CONTEXT, CALLER, msg.str());
            return;
        }

        // If the subroutine is currently being evaluated, temporarily store all the
        // existing local values, so that we can restore them later.
        TaggedValue previousLocals;
        if (function_t::get_attrs(function).currentlyEvaluating) {
            store_locals(functionBranch, &previousLocals);
        }

        function_t::get_attrs(function).currentlyEvaluating = true;

        // Load values into this function's stateful values. If this state has never been
        // saved then this function will reset this function's stateful values.
        Term* hiddenState = get_hidden_state_for_call(CALLER);

        // Subroutines only support Branches as state, if this state has a non-Branch type
        // then it was meant to execute as a builtin. As a workaround, we'll ignore a
        // non-Branch state so that we don't crash.
        if (hiddenState && !is_branch(hiddenState))
            hiddenState = NULL;

        if (hiddenState != NULL)
            load_state_into_branch(as_branch(hiddenState), functionBranch);

        // Load inputs into input placeholders.
        for (int input=0; input < num_inputs; input++) {

            std::string inputName = function_t::get_input_name(function, input);
            if (inputName == "#state") {
                ca_assert(input == 0);
                continue;
            }

            Term* term = function_t::get_input_placeholder(function, input);

            ca_assert(term->function == INPUT_PLACEHOLDER_FUNC);

            Term* incomingTerm = INPUT_TERM(input);
            if (term->type == ANY_TYPE)
                copy(incomingTerm, term);
            else
                cast(incomingTerm, term);
        }

        evaluate_branch(CONTEXT, functionBranch);

        // Clear interruptSubroutine flag
        bool returnCalled = CONTEXT->interruptSubroutine;
        CONTEXT->interruptSubroutine = false;

        // Hold a copy of output for now
        TaggedValue outputValue;

        // Copy output if no error occurred
        if (!CONTEXT->errorOccurred
                && functionBranch.length() > 0) {
            Term* output = functionBranch[functionBranch.length()-1];
            copy(output, &outputValue);
        }

        // Store state
        if (hiddenState != NULL)
            persist_state_from_branch(functionBranch, as_branch(hiddenState));

        // Restore locals, if needed
        if (is_null(&previousLocals)) {
            function_t::get_attrs(function).currentlyEvaluating = false;
        } else {
            restore_locals(&previousLocals, functionBranch);
        }

        if (returnCalled)
            copy(&CONTEXT->subroutineOutput, OUTPUT);
        else
            copy(&outputValue, OUTPUT);
    }
}

bool is_subroutine(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        && function_t::get_evaluate(term) == subroutine_t::evaluate;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    Branch& contents = sub->nestedContents;

    // If there is an #out term, then it needs to be the last term. If #out is a
    // name binding into an inner branch then this might not be the case
    if (contents.contains("#out") && contents[contents.length()-1]->name != "#out") {
        Term* copy = apply(contents, COPY_FUNC, RefList(contents["#out"]), "#out");
        set_source_hidden(copy, true);
    } else if (!contents.contains("#out")) {
        // If there's no #out term, then create an extra term to hold the output type
        Term* term = create_value(contents, outputType, "#out");
        set_source_hidden(term, true);
    }

    // If the #out term doesn't have the same type as the declared type, then cast it
    Term* outTerm = contents[contents.length()-1];
    if (outTerm->type != outputType) {
        outTerm = apply(contents, CAST_FUNC, RefList(outTerm), "#out");
        change_type(outTerm, outputType);
        set_source_hidden(outTerm, true);
    }

    // Install evaluate function
    function_t::get_evaluate(sub) = subroutine_t::evaluate;

    subroutine_update_hidden_state_type_from_contents(sub);
}

void subroutine_update_hidden_state_type_from_contents(Term* func)
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
        if (is_stateful(contents[i]))
            hasState = true;
        if (is_subroutine(contents[i]->function))
            if (is_function_stateful(contents[i]->function))
                hasState = true;
    }

    if (hasState)
        subroutine_change_state_type(func, BRANCH_TYPE);
}

void subroutine_change_state_type(Term* func, Term* newType)
{
    Term* previousType = function_t::get_hidden_state_type(func);
    if (previousType == newType)
        return;

    Branch& contents = func->nestedContents;
    function_t::get_attrs(func).hiddenStateType = newType;

    bool hasStateInput = (function_t::num_inputs(func) > 0)
        && (function_t::get_input_name(func, 0) == "#state");

    // create a stateful input if needed
    if (newType != VOID_TYPE && !hasStateInput) {
        contents.insert(1, alloc_term());
        rewrite(contents[1], INPUT_PLACEHOLDER_FUNC, RefList());
        contents.bindName(contents[1], "#state");
    }

    // If state was added, find all the calls to this function and insert
    // a state argument.
    if (previousType == VOID_TYPE && newType != VOID_TYPE) {
        for (BranchIterator it(contents); !it.finished(); ++it) {
            Term* term = *it;

            if (term->function == func) {
                Branch* branch = term->owningBranch;
                Term* stateContainer = alloc_term();
                branch->insert(term->index, stateContainer);
                change_type(stateContainer, function_t::get_hidden_state_type(func));
                change_function(stateContainer, STATEFUL_VALUE_FUNC);
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

bool is_subroutine_state_expanded(Term* term)
{
    ca_assert(term != NULL);
    return as_branch(term).length() > 0;
}

void expand_subroutines_hidden_state(Term* call, Term* state)
{
    ca_assert(is_subroutine(call->function));
    ca_assert(state != NULL);
    duplicate_branch(function_contents(call->function), as_branch(state));
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
