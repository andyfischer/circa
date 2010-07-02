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
            format_branch_source(source, as_branch(term), term);
    }

    CA_FUNCTION(evaluate)
    {
        Term* function = FUNCTION;
        Branch& functionBranch = as_branch(function);

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
        if (function_t::get_attrs(function).currentlyEvaluating)
            store_locals(functionBranch, &previousLocals);
        else
            function_t::get_attrs(function).currentlyEvaluating = true;

        // Load values into this function's stateful values. If this state has never been
        // saved then this function will reset this function's stateful values.
        Term* hiddenState = get_hidden_state_for_call(CALLER);

        if (hiddenState != NULL)
            load_state_into_branch(as_branch(hiddenState), functionBranch);

        // Load inputs into input placeholders.
        for (int input=0; input < num_inputs; input++) {

            std::string inputName = function_t::get_input_name(function, input);
            if (inputName == "#state") {
                assert(input == 0);
                continue;
            }

            Term* term = function_t::get_input_placeholder(function, input);

            assert(term->function == INPUT_PLACEHOLDER_FUNC);

            Term* incomingTerm = INPUT_TERM(input);
            if (term->type == ANY_TYPE)
                copy(incomingTerm, term);
            else
                cast(incomingTerm, term);
        }

        evaluate_branch(CONTEXT, functionBranch);

        // Copy output if no error occurred
        if (!CONTEXT->errorOccurred && functionBranch.length() > 0) {
            Term* output = functionBranch[functionBranch.length()-1];
            if (output->type != VOID_TYPE) {
                assert(output->name == "#out");
                copy(output, OUTPUT);
            }
        }

        // Store state
        if (hiddenState != NULL) {
            persist_state_from_branch(functionBranch, as_branch(hiddenState));
        }

        // Restore locals, if needed
        if (is_null(&previousLocals))
            function_t::get_attrs(function).currentlyEvaluating = false;
        else
            restore_locals(&previousLocals, functionBranch);
    }
}

bool is_subroutine(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        && function_t::get_evaluate(term) == subroutine_t::evaluate;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    Branch& contents = as_branch(sub);

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

    subroutine_update_hidden_state_type(sub);
}

void subroutine_update_hidden_state_type(Term* func)
{
    // Check if a stateful argument was declared
    Term* firstInput = function_t::get_input_placeholder(func, 0);
    if (firstInput != NULL && firstInput->boolPropOptional("state", false)) {
        function_t::get_hidden_state_type(func) = firstInput->type;
        return;
    }

    bool hasState = false;
    Branch& contents = as_branch(func);
    for (int i=0; i < contents.length(); i++) {
        if (contents[i] == NULL)
            continue;
        if (is_stateful(contents[i]))
            hasState = true;
        if (is_subroutine(contents[i]->function))
            if (is_function_stateful(contents[i]->function))
                hasState = true;
    }

    if (hasState) {
        function_t::get_hidden_state_type(func) = BRANCH_TYPE;
        bool alreadyHasStateInput = (function_t::num_inputs(func) > 0)
            && (function_t::get_input_name(func, 0) == "#state");
        if (!alreadyHasStateInput) {
            // Insert an input for state
            contents.insert(1, alloc_term());
            rewrite(contents[1], INPUT_PLACEHOLDER_FUNC, RefList());
            contents.bindName(contents[1], "#state");
        }
    } else {
        function_t::get_hidden_state_type(func) = VOID_TYPE;
    }
}

bool is_subroutine_state_expanded(Term* term)
{
    assert(term != NULL);
    return as_branch(term).length() > 0;
}

void expand_subroutines_hidden_state(Term* call, Term* state)
{
    assert(is_subroutine(call->function));
    assert(state != NULL);
    duplicate_branch(as_branch(call->function), as_branch(state));
}

void store_locals(Branch& branch, TaggedValue* storageTv)
{
    std::cout << "storing " << branch.length() << " locals" << std::endl;
    make_list(storageTv);
    List* storage = (List*) storageTv;
    storage->resize(branch.length());
    for (int i=0; i < branch.length(); i++) {
        copy(branch[i], storage->get(i));
    }
}

void restore_locals(TaggedValue* storageTv, Branch& branch)
{
    std::cout << "restoring " << branch.length() << " locals" << std::endl;
    List* storage = (List*) storageTv;
    for (int i=0; i < branch.length(); i++) {
        copy(storage->get(i), branch[i]);
    }
}

} // namespace circa
