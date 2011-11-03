// Copyright (c) Paul Hodge. See LICENSE file for license terms.
 
#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "building.h"
#include "builtins.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "locals.h"
#include "refactoring.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "update_cascades.h"
#include "token.h"
#include "term.h"
#include "type.h"

#include "subroutine.h"

namespace circa {

namespace subroutine_f {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, token::DEF);

        Function* func = as_function(term);

        function_format_header_source(source, func);

        if (!is_native_function(func))
            format_branch_source(source, nested_contents(term), term);
    }
}

Term* get_subroutine_input_placeholder(Branch* contents, int index)
{
    return contents->get(index);
}

CA_FUNCTION(evaluate_subroutine)
{
    EvalContext* context = CONTEXT;
    Term* caller = CALLER;
    Term* function = caller->function;
    Branch* contents = nested_contents(function);
    int numInputs = caller->numInputs();

    // Fetch state container
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);

    if (is_function_stateful(function))
        fetch_state_container(caller, &prevScopeState, &context->currentScopeState);

    // Fetch inputs and start preparing the new stack frame.
    List newFrame;
    newFrame.resize(get_locals_count(contents));
    
    context->interruptSubroutine = false;

    // Insert inputs into placeholders
    for (int i=0; i < NUM_INPUTS; i++) {
        Term* placeholder = get_subroutine_input_placeholder(contents, i);

        bool castSuccess = cast(INPUT(i), placeholder->type, newFrame[i]);

        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast input " << INPUT(i)->toString()
                << " (at index " << i << ")"
                << " to type " << placeholder->type->name;
            ERROR_OCCURRED(msg.str().c_str());
            return;
        }
    }

    // Prepare output
    set_null(&context->subroutineOutput);

    // Push our frame (with inputs) onto the stack
    push_frame(context, contents, &newFrame);

    // Evaluate each term
    for (int i=numInputs; i < contents->length(); i++) {
        evaluate_single_term(context, contents->get(i));
        if (evaluation_interrupted(context))
            break;
    }

    // If an error occurred, leave context the way it is.
    if (context->errorOccurred)
        return;

    // Save output
    TaggedValue output;

    Type* outputType = function_get_output_type(caller->function, 0);

    if (context->errorOccurred) {
        set_null(&output);
    } else if (outputType == &VOID_T) {

        set_null(&output);

    } else {

        bool castSuccess = cast(&context->subroutineOutput, outputType, &output);
        
        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast output " << context->subroutineOutput.toString()
                << " to type " << outputType->name;
            error_occurred(context, caller, &output, msg.str().c_str());
        }
    }

    set_null(&context->subroutineOutput);

    // Clean up
    pop_frame(context);
    swap(&output, OUTPUT);
    context->interruptSubroutine = false;
        
    // Preserve state
    if (is_function_stateful(function))
        save_and_consume_state(caller, &prevScopeState, &context->currentScopeState);
    swap(&context->currentScopeState, &prevScopeState);
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return as_function(term)->evaluate == evaluate_subroutine;
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
    as_function(sub)->evaluate = evaluate_subroutine;
    as_function(sub)->createsStackFrame = true;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    subroutine_check_to_append_implicit_return(sub);
    finish_update_cascade(nested_contents(sub));
}

void subroutine_check_to_append_implicit_return(Term* sub)
{
    // Do nothing if this subroutine already ends with a return
    Branch* contents = nested_contents(sub);
    for (int i=contents->length()-1; i >= 0; i--) {
        Term* term = contents->get(i);
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

void store_locals(Branch* branch, TaggedValue* storageTv)
{
    touch(storageTv);
    set_list(storageTv);
    List* storage = List::checkCast(storageTv);
    storage->resize(branch->length());
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

        if (term == NULL) continue;

        if (term->type == &FUNCTION_ATTRS_T)
            continue;

        copy(term, storage->get(i));
    }
}

void restore_locals(TaggedValue* storageTv, Branch* branch)
{
    if (!is_list(storageTv))
        internal_error("storageTv is not a list");

    List* storage = List::checkCast(storageTv);

    // The function branch may be longer than our list of locals. 
    int numItems = storage->length();
    for (int i=0; i < numItems; i++) {
        Term* term = branch->get(i);

        if (term == NULL) continue;

        if (term->type == &FUNCTION_ATTRS_T)
            continue;

        copy(storage->get(i), term);
    }
}

void call_subroutine(Branch* sub, TaggedValue* inputs, TaggedValue* output,
                     TaggedValue* error)
{
    internal_error("call_subroutine no worky");
}

void call_subroutine(Term* sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error)
{
    return call_subroutine(nested_contents(sub), inputs, output, error);
}

} // namespace circa
