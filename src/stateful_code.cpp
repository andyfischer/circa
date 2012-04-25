// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "function.h"
#include "if_block.h"
#include "introspection.h"
#include "source_repro.h"
#include "subroutine.h"
#include "stateful_code.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

static void branch_update_existing_pack_state_calls(Branch* branch);
static void get_list_of_state_outputs(Branch* branch, int position, TermList* output);
static void append_final_pack_state(Branch* branch);

bool is_declared_state(Term* term)
{
    return term->function == FUNCS.declared_state;
}

bool has_implicit_state(Term* term)
{
    return has_state_input(term_get_function_details(term));
}

bool is_function_stateful(Term* func)
{
    if (!is_function(func))
        return false;
    Function* attrs = as_function(func);
    if (attrs == NULL)
        return false;

    return function_has_state_input(attrs);
}

void pack_any_open_state_vars(Branch* branch)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;
        if (term->function == FUNCS.declared_state) {
            Term* result = branch->get(term->name);

            // If this result already has a pack_state() term then leave it alone.
            if (find_user_with_function(result, FUNCS.pack_state) != NULL)
                continue;

            Term* pack = apply(branch, FUNCS.pack_state, TermList(
                find_open_state_result(branch, branch->length()), result, term));
            pack->setStringProp("field", unique_name(term));
            branch->move(pack, result->index + 1);
        }
    }
}

bool branch_state_type_is_out_of_date(Branch* branch)
{
    // Alloc an array that tracks, for each field in the existing stateType,
    // whether we have found a corresponding term for that field.
    bool* typeFieldFound = NULL;
    int existingFieldCount = 0;

    if (branch->stateType != NULL) {
        existingFieldCount = compound_type_get_field_count(branch->stateType);
        size_t size = sizeof(bool) * existingFieldCount;
        typeFieldFound = (bool*) malloc(size);
        memset(typeFieldFound, 0, size);
    }
    
    // Walk through every term and check whether every unpack_state call is already
    // mentioned in the state type.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->function != FUNCS.unpack_state)
            continue;

        // Found an unpack_state call
        Term* identifyingTerm = term->input(1);

        // If the branch doesn't yet have a stateType then that's an update.
        if (branch->stateType == NULL)
            goto return_true;

        // Look for the field name
        int fieldIndex = list_find_field_index_by_name(branch->stateType,
            unique_name(identifyingTerm));

        // If the name isn't found then that's an update
        if (fieldIndex == -1)
            goto return_true;

        // If the type doesn't match then that's an update
        if (compound_type_get_field_type(branch->stateType, fieldIndex)
                != declared_type(term))
            goto return_true;

        // Record this field index as 'found'
        typeFieldFound[fieldIndex] = true;
    }

    // If there were any fields in the type that weren't found in the branch, then
    // that's an update.
    if (typeFieldFound != NULL) {
        for (int i=0; i < existingFieldCount; i++) {
            if (!typeFieldFound[i])
                goto return_true;
        }
    }

    // No reason to update, return false.
    free(typeFieldFound);
    return false;

return_true:
    free(typeFieldFound);
    return true;
}

void branch_update_state_type(Branch* branch)
{
    if (branch_state_type_is_out_of_date(branch)) {

        // TODO: Handle the case where the stateType should go from non-NULL to NULL

        // Recreate the state type
        Type* type = create_compound_type();

        // TODO: give this new type a nice name

        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);
            if (term == NULL)
                continue;

            if (term->function != FUNCS.unpack_state)
                continue;

            Term* identifyingTerm = term->input(1);

            compound_type_append_field(type, declared_type(term), unique_name(identifyingTerm));
        }

        branch->stateType = type;

        // Might need to update any existing pack_state calls.
        branch_update_existing_pack_state_calls(branch);
    }
}

static void append_final_pack_state(Branch* branch)
{
    TermList inputs;
    get_list_of_state_outputs(branch, branch->length(), &inputs);
    Term* term = apply(branch, FUNCS.pack_state, inputs);
    term->setBoolProp("final", true);
}

// For the given field name
static Term* find_output_term_for_state_field(Branch* branch, const char* fieldName, int position)
{
    Term* result = find_from_unique_name(branch, fieldName);

    // For declared state, the result is the last term with the given name
    if (result->function == FUNCS.declared_state) {
        return find_local_name(branch, result->name.c_str(), position);
    }

    ca_assert(result != NULL);

    // This term might be the actual state result, or the state result might be
    // found in an extra output. Look around and see if this term has a stateful
    // extra output.
    for (int outputIndex=0;; outputIndex++) {
        Term* extraOutput = get_extra_output(result, outputIndex);
        if (extraOutput == NULL)
            break;

        if (is_state_output(extraOutput))
            return extraOutput;
    }

    return result;
}

// Write a list of terms to 'output' corresponding to the list of state outputs at this
// position in the branch. Useful for populating a list of inputs for pack_state.
static void get_list_of_state_outputs(Branch* branch, int position, TermList* output)
{
    output->clear();

    if (branch->stateType == NULL)
        return;

    for (int i=0; i < compound_type_get_field_count(branch->stateType); i++) {

        const char* fieldName = compound_type_get_field_name(branch->stateType, i);
        Term* result = find_output_term_for_state_field(branch, fieldName, position);
        output->append(result);
    }
}

static void branch_update_existing_pack_state_calls(Branch* branch)
{
    if (branch->stateType == NULL) {
        // No state type, make sure there's no pack_state call.
        // TODO: Handle this case properly (should search and destroy an existing pack_state call)
        return;
    }

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term == NULL)
            continue;

        if (term->function != FUNCS.pack_state)
            continue;

        // Update the inputs for this pack_state call
        TermList inputs;
        get_list_of_state_outputs(branch, i, &inputs);

        set_inputs(term, inputs);
    }
}

Term* find_or_create_state_input(Branch* branch)
{
    // check if there is already a stateful input
    Term* existing = find_state_input(branch);
    if (existing != NULL)
        return existing;

    // None yet, insert one
    Term* input = append_state_input(branch);

    // Add a final pack_state call too
    append_final_pack_state(branch);

    return input;
}

Term* branch_add_pack_state(Branch* branch)
{
    TermList inputs;
    get_list_of_state_outputs(branch, branch->length(), &inputs);

    // Don't create anything if there are no state outputs
    if (inputs.length() == 0)
        return NULL;

    return apply(branch, FUNCS.pack_state, inputs);
}

// Unpack a state value. Input 1 is the "identifying term" which is used as a key.
void unpack_state(caStack* stack)
{
    caValue* container = circa_input(stack, 0);
    Term* identifyingTerm = (Term*) circa_caller_input_term(stack, 1);

    caValue* element = get_field(container, unique_name(identifyingTerm));

    if (element == NULL) {
        set_null(circa_output(stack, 0));
    } else {
        copy(element, circa_output(stack, 0));
    }
}

// Pack a state value. Each input will correspond with a slot in the branch's state type.
void pack_state(caStack* stack)
{
    Term* caller = (Term*) circa_caller_term(stack);
    Branch* branch = caller->owningBranch;

    if (branch->stateType == NULL)
        return;

    caValue* args = circa_input(stack, 0);
    caValue* output = circa_output(stack, 0);
    create(branch->stateType, output);

    for (int i=0; i < circa_count(args); i++) {
        caValue* input = circa_index(args, i);
        caValue* dest = list_get(output, i);
        if (input == NULL)
            set_null(dest);
        else
            copy(input, dest);
    }
}

} // namespace circa
