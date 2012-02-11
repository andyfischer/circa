// Copyright (c) Paul Hodge. See LICENSE file for license terms.
 
#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "locals.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "names.h"
#include "update_cascades.h"
#include "token.h"
#include "term.h"
#include "type.h"

#include "subroutine.h"

namespace circa {

namespace subroutine_f {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, TK_DEF);

        Function* func = as_function(term);

        function_format_header_source(source, func);

        if (!is_native_function(func))
            format_branch_source(source, nested_contents(term), term);
    }
}

CA_FUNCTION(evaluate_subroutine)
{
    EvalContext* context = CONTEXT;
    Term* caller = CALLER;
    Term* function = caller->function;

    Branch* contents = NULL;

    if (caller->nestedContents != NULL)
        contents = nested_contents(caller);
    else
        contents = nested_contents(function);
    
    // Fetch inputs and start preparing the new stack frame.
    List registers;
    registers.resize(get_locals_count(contents));
    
    // Insert inputs into placeholders
    for (int i=0; i < NUM_INPUTS; i++) {
        Term* placeholder = get_input_placeholder(contents, i);
        if (placeholder == NULL)
            break;

        bool castSuccess = cast(INPUT(i), placeholder->type, registers[i]);

        if (!castSuccess) {
            std::stringstream msg;
            msg << "Couldn't cast input " << INPUT(i)->toString()
                << " (at index " << i << ")"
                << " to type " << name_to_string(placeholder->type->name),
            RAISE_ERROR(msg.str().c_str());
            return;
        }
    }

    // Push our frame (with inputs) onto the stack
    push_frame(context, contents, &registers);
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return as_function(term)->evaluate == evaluate_subroutine;
}

Term* find_enclosing_subroutine(Term* term)
{
    while (true) {
        Term* parent = get_parent_term(term);
        if (parent == NULL)
            return NULL;
        if (is_subroutine(parent))
            return parent;

        term = parent;
    }
}

int get_input_index_of_placeholder(Term* inputPlaceholder)
{
    ca_assert(inputPlaceholder->function == INPUT_PLACEHOLDER_FUNC);
    return inputPlaceholder->index - 1;
}

Term* create_subroutine(Branch* branch, const char* name)
{
    Term* term = create_value(branch, &FUNCTION_T, name);
    initialize_function(term);
    initialize_subroutine(term);
    return term;
}

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    as_function(sub)->evaluate = evaluate_subroutine;
    as_function(sub)->createsStackFrame = true;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    finish_update_cascade(nested_contents(sub));
}

void store_locals(Branch* branch, TValue* storageTv)
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

void restore_locals(TValue* storageTv, Branch* branch)
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

void call_subroutine(Branch* sub, TValue* inputs, TValue* output,
                     TValue* error)
{
    internal_error("call_subroutine no worky");
}

void call_subroutine(Term* sub, TValue* inputs, TValue* output, TValue* error)
{
    return call_subroutine(nested_contents(sub), inputs, output, error);
}

} // namespace circa
