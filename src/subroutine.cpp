// Copyright (c) Paul Hodge. See LICENSE file for license terms.
 
#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "building.h"
#include "builtins.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "locals.h"
#include "refactoring.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "token.h"
#include "term.h"
#include "type.h"

#include "subroutine.h"

namespace circa {

namespace subroutine_f {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, token::DEF);

        function_t::format_header_source(source, term);

        if (!is_native_function(term))
            format_branch_source(source, nested_contents(term), term);
    }
}

Term* get_subroutine_input_placeholder(Branch& contents, int index)
{
    return contents[index + 1];
}

Term* get_subroutine_output_type(Branch& contents)
{
    return as_function_attrs(contents[0]).outputTypes[0];
}

void evaluate_subroutine_internal(EvalContext* context, Term* caller,
        Branch& contents, List* inputs, List* outputs)
{
    context->interruptSubroutine = false;
    context->stack.append(caller);
    start_using(contents);

    int numInputs = inputs->length();

    // Insert inputs into placeholders
    for (int i=0; i < numInputs; i++) {
        Term* placeholder = get_subroutine_input_placeholder(contents, i);
        swap(inputs->get(i), get_local(placeholder));
    }

    // Prepare output
    set_null(&context->subroutineOutput);

    // Evaluate each term
    for (int i=numInputs+1; i < contents.length(); i++) {
        evaluate_single_term(context, contents[i]);
        if (evaluation_interrupted(context))
            break;
    }

    // Fetch output
    Term* outputTypeTerm = get_subroutine_output_type(contents);
    Type* outputType = unbox_type(outputTypeTerm);

    ca_assert(is_list(&context->subroutineOutput) || is_null(&context->subroutineOutput));

    if (is_list(&context->subroutineOutput))
        swap(&context->subroutineOutput, outputs);
    
    set_null(&context->subroutineOutput);

    if (context->errorOccurred) {
        outputs->resize(1);
        set_null(outputs->get(0));
    } else if (outputTypeTerm == VOID_TYPE) {

        set_null(outputs->get(0));

    } else {

        TaggedValue* output0 = outputs->get(0);

        bool castSuccess = cast(output0, outputType, outputs->get(0));
        
        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast output " << output0->toString()
                << " to type " << outputType->name;
            error_occurred(context, caller, msg.str());
        }
    }

    // Rescue input values
    for (int i=0; i < numInputs; i++) {
        Term* placeholder = get_subroutine_input_placeholder(contents, i);
        swap(inputs->get(i), get_local(placeholder));
    }

    // Clean up
    finish_using(contents);
    context->stack.pop();
    context->interruptSubroutine = false;
}

void evaluate_subroutine(EvalContext* context, Term* caller)
{
    Term* function = caller->function;
    Branch& contents = nested_contents(function);
    int numInputs = caller->numInputs();

    // Copy inputs to a temporary list
    List inputs;
    inputs.resize(numInputs);

    for (int i=0; i < numInputs; i++) {
        if (caller->input(i) == NULL)
            continue;

        TaggedValue* input = get_input(caller, i);

        Type* inputType = unbox_type(function_t::get_input_type(function, i));

        bool success = cast(input, inputType, inputs[i]);
        if (!success) {
            std::stringstream msg;
            msg << "Couldn't cast input " << i << " to " << inputType->name;
            return error_occurred(context, caller, msg.str());
        }
    }

    // Fetch state container
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);

    if (is_function_stateful(function))
        fetch_state_container(caller, &prevScopeState, &context->currentScopeState);

    // Evaluate contents
    List outputs;
    evaluate_subroutine_internal(context, caller, contents, &inputs, &outputs);

    // Preserve state
    if (is_function_stateful(function))
        save_and_consume_state(caller, &prevScopeState, &context->currentScopeState);
    swap(&context->currentScopeState, &prevScopeState);

    // Write output
    TaggedValue* outputDest = get_output(caller, 0);
    if (outputDest != NULL)
        swap(outputs[0], outputDest);

    // Write extra outputs
    ca_assert(outputs.length() == get_output_count(caller));

    for (int i=1; i < outputs.length(); i++)
        copy(outputs[i], get_output(caller, i));
}

bool is_subroutine(Term* term)
{
    if (term->type != FUNCTION_TYPE)
        return false;
    if (!has_nested_contents(term))
        return false;
    if (nested_contents(term).length() < 1)
        return false;
    if (term->contents(0)->type != FUNCTION_ATTRS_TYPE)
        return false;
    return function_t::get_evaluate(term) == evaluate_subroutine;
}

Term* find_enclosing_subroutine(Term* term)
{
    Term* parent = get_parent_term(term);
    if (parent == NULL)
        return NULL;
    if (is_subroutine(parent))
        return parent;
    return find_enclosing_subroutine(parent);
}

int get_input_index_of_placeholder(Term* inputPlaceholder)
{
    ca_assert(inputPlaceholder->function == INPUT_PLACEHOLDER_FUNC);
    return inputPlaceholder->index - 1;
}

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    function_t::get_evaluate(sub) = evaluate_subroutine;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    subroutine_update_state_type_from_contents(sub);
    subroutine_check_to_append_implicit_return(sub);
}

void subroutine_update_state_type_from_contents(Term* func)
{
    // Check if a stateful argument was declared
    Term* firstInput = function_t::get_input_placeholder(func, 0);
    if (firstInput != NULL && firstInput->boolPropOptional("state", false)) {
        // already updated state
        return;
    }

    if (has_implicit_state(func))
        subroutine_change_state_type(func, LIST_TYPE);
}

void subroutine_change_state_type(Term* func, Term* newType)
{
    FunctionAttrs* attrs = get_function_attrs(func);
    Term* previousType = attrs->implicitStateType;
    if (previousType == newType)
        return;

    Branch& contents = nested_contents(func);
    attrs->implicitStateType = newType;

    bool hasStateInput = (function_t::num_inputs(func) > 0)
        && (function_t::get_input_name(func, 0) == "#state");

    // create a stateful input if needed
    if (newType != VOID_TYPE && !hasStateInput) {
        contents.insert(1, alloc_term());
        Term* input = contents[1];
        rewrite(input, INPUT_PLACEHOLDER_FUNC, TermList());
        contents.bindName(input, "#state");
        input->setBoolProp("optional", true);
    }

    // If state was added, find all the recursive calls to this function and
    // insert a state argument.
    if (previousType == VOID_TYPE && newType != VOID_TYPE) {
        for (BranchIterator it(&contents); !it.finished(); ++it) {
            Term* term = *it;

            if (term->function == func)
                insert_input(term, NULL);
        }
    }
}

void subroutine_check_to_append_implicit_return(Term* sub)
{
    // Do nothing if this subroutine already ends with a return
    Branch& contents = nested_contents(sub);
    for (int i=contents.length()-1; i >= 0; i--) {
        Term* term = contents[i];
        if (term->function == RETURN_FUNC)
            return;

        // if we found a comment() then keep searching
        if (term->function == COMMENT_FUNC)
            continue;

        // otherwise, break so that we'll insert a return()
        break;
    }

    post_compile_term(apply(contents, RETURN_FUNC, TermList(NULL)));
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

void call_subroutine(Branch& sub, TaggedValue* inputs, TaggedValue* output,
                     TaggedValue* error)
{
    List* inputsList = List::checkCast(inputs);
    ca_assert(inputsList != NULL);

    EvalContext context;
    List outputs;
    evaluate_subroutine_internal(&context, NULL, sub, inputsList, &outputs);

    // TODO: caller should get this whole list
    copy(outputs.get(0), output);

    if (context.errorOccurred)
        set_string(error, context.errorMessage);
}

void call_subroutine(Term* sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error)
{
    return call_subroutine(nested_contents(sub), inputs, output, error);
}

} // namespace circa
