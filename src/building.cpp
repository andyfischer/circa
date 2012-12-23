// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "building.h"
#include "interpreter.h"
#include "function.h"
#include "heap_debugging.h"
#include "if_block.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "loops.h"
#include "names.h"
#include "parser.h"
#include "names.h"
#include "selector.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

void on_term_created(Term* term)
{
    // debugging hook
}

Term* apply(Block* block, Term* function, TermList const& inputs, Name name)
{
    block_start_changes(block);

    // If function is NULL, use 'unknown_function' instead.
    if (function == NULL)
        function = FUNCS.unknown_function;

    // If 'function' is actually a type, create a call to cast().
    if (function != NULL && is_type(function)) {
        Term* term = apply(block, FUNCS.cast, inputs);
        change_declared_type(term, as_type(term_value(function)));
        return term;
    }

    // Figure out the term position; it should be placed before any output() terms.
    // (unless it's an output term itself).
    int position = block->length();
    if (function != FUNCS.output) {
        while (position > 0) {
            Term* preceding = block->get(position - 1);
            if (preceding == NULL)
                break;

            // Position before output() terms.
            if (preceding->function == FUNCS.output) {
                position--;
                continue;
            }

            // Position before a pack_state() call that is 'final'
            if (preceding->function == FUNCS.pack_state
                    && preceding->boolProp("final", false)) {
                position--;
                continue;
            }

            break;
        }
    }

    // Create the term
    Term* term = block->appendNew();
    INCREMENT_STAT(TermsCreated);

    on_term_created(term);

    // Position the term before any output_placeholder terms.
    block->move(term, position);

    if (name != name_None)
        rename(term, name);

    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);

    // update term->function, change_function will also update the declared type.
    change_function(term, function);

    update_unique_name(term);
    update_extra_outputs(term);

    // Post-compile steps

    // Possibly run the function's postCompile handler
    if (is_function(function)) {
        Function::PostCompile func = as_function(term_value(function))->postCompile;

        if (func != NULL)
            func(term);
    }

    return term;
}

void set_input(Term* term, int index, Term* input)
{
    block_start_changes(term->owningBlock);

    assert_valid_term(term);
    assert_valid_term(input);

    Term* previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    while (index >= term->numInputs())
        term->inputs.push_back(NULL);

    term->inputs[index].term = input;

    // Add 'term' to the user list of 'input'
    append_user(term, input);

    // Check if we should remove 'term' from the user list of previousInput
    possibly_prune_user_list(term, previousInput);
}

void set_inputs(Term* term, TermList const& inputs)
{
    assert_valid_term(term);

    for (int i=0; i < term->numInputs(); i++) {
        assert_valid_term(term->input(i));
    }

    Term::InputList previousInputs = term->inputs;

    term->inputs.resize(inputs.length());
    for (int i=0; i < inputs.length(); i++) {
        assert_valid_term(inputs[i]);
        term->inputs[i] = Term::Input(inputs[i]);
    }

    // Add 'term' as a user to these new inputs
    for (int i=0; i < inputs.length(); i++)
        append_user(term, inputs[i]);

    // Check to remove 'term' from user list of any previous inputs
    for (size_t i=0; i < previousInputs.size(); i++)
        possibly_prune_user_list(term, previousInputs[i].term);
}

void insert_input(Term* term, Term* input)
{
    term->inputs.insert(term->inputs.begin(), Term::Input(NULL));
    set_input(term, 0, input);
}

void insert_input(Term* term, int index, Term* input)
{
    term->inputs.insert(term->inputs.begin() + index, Term::Input(NULL));
    set_input(term, index, input);
}

void append_user(Term* user, Term* usee)
{
    if (usee == NULL)
        return;

    int originalUserCount = user_count(usee);

    if (usee != NULL && user != NULL)
        usee->users.appendUnique(user);

    // Check if we added the first user.
    if (originalUserCount == 0 && user_count(usee) > 0) {

        // for-loop bytecode depends on the user count.
        if (usee->function == FUNCS.for_func)
            dirty_bytecode(usee->nestedContents);
    }
}

static void remove_user(Term* usee, Term* user)
{
    int originalUserCount = user_count(usee);

    usee->users.remove(user);

    // Check if we removed the last user.
    if (originalUserCount > 0 && user_count(usee) == 0) {

        // for-loop bytecode depends on the user count.
        if (usee->function == FUNCS.for_func)
            dirty_bytecode(usee->nestedContents);
    }
    
}

void possibly_prune_user_list(Term* user, Term* usee)
{
    if (usee == NULL)
        return;

    if (!is_actually_using(user, usee))
        remove_user(usee, user);
}

void remove_from_any_user_lists(Term* term)
{
    for (int i=0; i < term->numDependencies(); i++) {
        Term* usee = term->dependency(i);
        assert_valid_term(usee);
        if (usee == NULL)
            continue;

        remove_user(usee, term);
    }
}

void clear_from_dependencies_of_users(Term* term)
{
    // Make a local copy of 'users', because these calls will want to modify
    // the list.
    TermList users = term->users;

    term->users.clear();

    for (int i=0; i < users.length(); i++) {
        Term* user = users[i];
        for (int input=0; input < user->numInputs(); input++)
            if (user->input(input) == term)
                set_input(user, input, NULL);
        if (user->function == term)
            change_function(user, NULL);
    }
}

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    Term* previousFunction = term->function;

    term->function = function;

    possibly_prune_user_list(term, previousFunction);

    on_create_call(term);

    respecialize_type(term);

    // Don't append user for certain functions. Need to make this more robust.
    if (function != NULL
            && function != FUNCS.value
            && function != FUNCS.input) {
        append_user(term, function);
    }

    // Possibly insert a state input for the enclosing subroutine.
    if (is_function_stateful(function))
        find_or_create_state_container(term->owningBlock);

    dirty_bytecode(term->owningBlock);
}

void change_declared_type(Term *term, Type *newType)
{
    // Don't allow 'null' to be used as a declared type (use 'any' instead)
    if (newType == TYPES.null)
        newType = TYPES.any;

    ca_assert(term != NULL);
    ca_assert(newType != NULL);

    if (term->type == newType)
        return;

    term->type = newType;

    set_null(term_value(term));

    // TODO: Use update_cascades to update inferred type on all users.
}

void respecialize_type(Term* term)
{
    Type* outputType = derive_specialized_output_type(term->function, term);
    if (outputType != term->type)
        change_declared_type(term, outputType);
}

void rename(Term* termToRename, Name name)
{
    // No-op if term already has this name.
    if (termToRename->nameSymbol == name)
        return;

    Block* block = termToRename->owningBlock;

    // Update binding in the owning block.
    if (block != NULL) {
        if (!has_empty_name(termToRename)) {
            termToRename->owningBlock->names.remove(termToRename->name);
            termToRename->name = "";
            termToRename->nameSymbol = name_None;
        }
        termToRename->owningBlock->bindName(termToRename, name);
    }

    // Update name symbol.
    termToRename->nameSymbol = name;
    termToRename->name = name_to_string(name);
    update_unique_name(termToRename);

    // Update unique ordinal. If any neighbor term has the same name, then give this
    // term a greater ordinal value.
    termToRename->uniqueOrdinal = 0;

    if (block != NULL) {
        for (int i=0; i < block->length(); i++) {
            Term* neighbor = block->get(i);
            if (neighbor == termToRename)
                continue;
            if (neighbor == NULL)
                continue;

            if (neighbor->nameSymbol == name) {
                // Check if the neighbor has ordinal value 0 (meaning no name collision).
                // If so, then promote it to 1 (meaning there is a collision.
                if (neighbor->uniqueOrdinal == 0)
                    neighbor->uniqueOrdinal = 1;

                if (neighbor->uniqueOrdinal >= termToRename->uniqueOrdinal)
                    termToRename->uniqueOrdinal = neighbor->uniqueOrdinal + 1;
            }
        }
    }

    // Handle change cascades.

    // Possibly store the shadowed name binding (this is only computed if needed)
    bool foundShadowedNameBinding = false;
    Term* shadowedNameBinding = NULL;
    
    if (name != name_None) {
        // The new name may have shadowed an existing name.
        for (NameVisibleIterator it(termToRename); it.unfinished(); ++it) {
            Term* possibleUser = it.current();
            for (int i=0; i < possibleUser->numInputs(); i++) {

                // Only look at inputs that have our name binding.
                if (possibleUser->input(i) == NULL)
                    continue;
                if (possibleUser->input(i)->nameSymbol != name)
                    continue;

                if (!foundShadowedNameBinding) {
                    foundShadowedNameBinding = true;
                    shadowedNameBinding = find_name_at(termToRename, name);
                    // shadowedNameBinding might still be NULL.
                }

                // We found a term that is using the name that we just rebound,
                // and the term is at a location where our new name should be visible.
                // So, this term is a candidate for a rebinding from shadowing.
                //
                // We're going to be conservative, and only touch bindings that were
                // bound to the previous name binding at this location. There probably
                // shouldn't be any other kind of binding, but we're still refining
                // things.
                
                if (possibleUser->input(i) == shadowedNameBinding) {
                    remap_pointers_quick(possibleUser, possibleUser->input(i), termToRename);
                }
            }
        }
    }
}

Term* create_duplicate(Block* block, Term* original, std::string const& name)
{
    ca_assert(original != NULL);

    TermList inputs;
    original->inputsToList(inputs);

    Term* term = apply(block, original->function, inputs, name_from_string(name));
    change_declared_type(term, original->type);

    copy(term_value(original), term_value(term));

    term->sourceLoc = original->sourceLoc;
    copy(&original->properties, &term->properties);

    // Special case for certain types, update declaringTerm
    if (is_type(term))
        as_type(term_value(term))->declaringTerm = term;
    if (is_function(term))
        as_function(term_value(term))->declaringTerm = term;

    return term;
}

Term* apply(Block* block, std::string const& functionName, TermList const& inputs, std::string const& name)
{
    Term* function = find_name(block, functionName.c_str());
    if (function == NULL)
        internal_error("function not found: "+functionName);

    Term* result = apply(block, function, inputs, name_from_string(name));
    result->setStringProp("syntax:functionName", functionName.c_str());
    return result;
}

Term* create_value(Block* block, Type* type, std::string const& name)
{
    // This function is safe to call while bootstrapping.
    ca_assert(type != NULL);

    Term *term = apply(block, FUNCS.value, TermList(), name_from_string(name));

    change_declared_type(term, type);
    make(type, term_value(term));

    if (type == TYPES.type) {
        set_string(&as_type(term)->name, name.c_str());
        as_type(term)->declaringTerm = term;
    }

    return term;
}

Term* create_value(Block* block, std::string const& typeName, std::string const& name)
{
    Term* type = NULL;

    type = find_name(block, typeName.c_str());

    if (type == NULL)
        internal_error("Couldn't find type: "+typeName);

    return create_value(block, as_type(term_value(type)), name);
}

Term* create_value(Block* block, caValue* initialValue, std::string const& name)
{
    Term* term = create_value(block, initialValue->value_type, name);
    copy(initialValue, term_value(term));
    return term;
}

Term* create_string(Block* block, std::string const& s, std::string const& name)
{
    Term* term = create_value(block, TYPES.string, name);
    set_string(term_value(term), s);
    return term;
}

Term* create_int(Block* block, int i, std::string const& name)
{
    Term* term = create_value(block, TYPES.int_type, name);
    set_int(term_value(term), i);
    return term;
}

Term* create_float(Block* block, float f, std::string const& name)
{
    Term* term = create_value(block, TYPES.float_type, name);
    set_float(term_value(term), f);
    return term;
}

Term* create_bool(Block* block, bool b, std::string const& name)
{
    Term* term = create_value(block, TYPES.bool_type, name);
    set_bool(term_value(term), b);
    return term;
}

Term* create_void(Block* block, std::string const& name)
{
    return create_value(block, TYPES.void_type, name);
}

Term* create_list(Block* block, std::string const& name)
{
    Term* term = create_value(block, TYPES.list, name);
    return term;
}

Block* create_block(Block* owner, const char* name)
{
    return nested_contents(apply(owner, FUNCS.section_block, TermList(), name_from_string(name)));
}

Block* find_or_create_block(Block* owner, const char* name)
{
    Term* existing = find_local_name(owner, name);
    if (existing != NULL)
        return nested_contents(existing);
    return create_block(owner, name);
}

Block* create_namespace(Block* block, std::string const& name)
{
    return apply(block, FUNCS.namespace_func, TermList(), name_from_string(name))->contents();
}
Block* create_block_unevaluated(Block* owner, const char* name)
{
    return nested_contents(apply(owner, FUNCS.block_unevaluated, TermList(), name_from_string(name)));
}

Term* create_type(Block* block, std::string nameStr)
{
    Term* term = create_value(block, TYPES.type);

    if (nameStr != "") {
        set_string(&as_type(term_value(term))->name, nameStr.c_str());
        rename(term, name_from_string(nameStr));
    }

    return term;
}

Term* create_type_value(Block* block, Type* value, std::string const& name)
{
    Term* term = create_value(block, TYPES.type, name);
    set_type(term_value(term), value);

    if (value->declaringTerm == NULL)
        value->declaringTerm = term;
        
    return term;
}

Term* duplicate_value(Block* block, Term* term)
{
    Term* dup = create_value(block, term->type);
    copy(term_value(term), term_value(dup));
    return dup;
}

Term* append_input_placeholder(Block* block)
{
    int count = count_input_placeholders(block);
    Term* term = apply(block, FUNCS.input, TermList());
    block->move(term, count);
    return term;
}
Term* append_output_placeholder(Block* block, Term* result)
{
    int count = count_output_placeholders(block);
    Term* term = apply(block, FUNCS.output, TermList(result));
    block->move(term, block->length() - count - 1);
    return term;
}
Term* prepend_output_placeholder(Block* block, Term* result)
{
    return apply(block, FUNCS.output, TermList(result));
}

Term* append_state_input(Block* block)
{
    // Make sure that a state input doesn't already exist
    Term* existing = find_state_input(block);
    if (existing != NULL)
        return existing;

    int inputCount = count_input_placeholders(block);

    Term* term = apply(block, FUNCS.input, TermList());
    block->move(term, inputCount);
    term->setBoolProp("state", true);
    term->setBoolProp("hiddenInput", true);
    term->setBoolProp("output", true);

    on_block_inputs_changed(block);

    return term;
}

Term* append_state_output(Block* block)
{
    // Make sure that a state input doesn't already exist
    Term* existing = find_state_output(block);
    if (existing != NULL)
        return existing;

    Term* term = append_output_placeholder(block, 
        find_open_state_result(block, block->length()));
    term->setBoolProp("state", true);
    hide_from_source(term);
    return term;
}

void update_extra_outputs(Term* term)
{
    Block* block = term->owningBlock;
    Block* function = term_get_function_details(term);

    if (function == NULL)
        return;

    bool needToUpdatePackState = false;

    for (int index=1; ; index++) {
        Term* placeholder = get_output_placeholder(function, index);
        if (placeholder == NULL)
            break;

        Name name = name_None;

        // Find the associated input placeholder (if any).
        int rebindsInput = placeholder->intProp("rebindsInput", -1);

        if (rebindsInput == -1) {

            // Check if we can find the rebind-input by looking for state input.
            if (is_state_output(placeholder))
                rebindsInput = find_state_input(function)->index;
        }

        // Use the appropriate name
        if (rebindsInput >= 0) {
            Term* input = term->input(rebindsInput);

            if (input != NULL)
                name = input->nameSymbol;

        } else {
            name = placeholder->nameSymbol;
        }

        Term* extra_output = NULL;

        // Check if this extra_output() already exists
        Term* existingSlot = block->getSafe(term->index + index);
        if (existingSlot != NULL && existingSlot->function == FUNCS.extra_output)
            extra_output = existingSlot;
        
        if (extra_output == NULL) {
            extra_output = apply(block, FUNCS.extra_output, TermList(term), name);
            move_to_index(extra_output, term->index + index);

            if (rebindsInput >= 0)
                extra_output->setIntProp("rebindsInput", rebindsInput);

            if (is_state_input(placeholder))
                needToUpdatePackState = true;
        }

        change_declared_type(extra_output, placeholder->type);

        if (is_state_input(placeholder))
            extra_output->setBoolProp("state", true);
    }

    if (needToUpdatePackState)
        block_update_existing_pack_state_calls(block);
}

Term* find_open_state_result(Block* block, int position)
{
    for (int i = position - 1; i >= 0; i--) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;
        if (term->function == FUNCS.input && is_state_input(term))
            return term;
        if (term->function == FUNCS.pack_state
                || term->function == FUNCS.pack_state_list_n)
            return term;
    }
    return NULL;
}

Term* find_open_state_result(Term* location)
{
    return find_open_state_result(location->owningBlock, location->index);
}

void check_to_insert_implicit_inputs(Term* term)
{
    if (!is_function(term->function))
        return;

    Term* stateInput = find_active_state_container(term_get_function_details(term));

    if (stateInput != NULL && !term_is_state_input(term, stateInput->index)) {

        int inputIndex = stateInput->index;

        Term* container = find_or_create_state_container(term->owningBlock);

        // Add a unpack_state() call
        Term* unpack = apply(term->owningBlock, FUNCS.unpack_state,
            TermList(container, term));
        hide_from_source(unpack);
        term->owningBlock->move(unpack, term->index);

        insert_input(term, inputIndex, unpack);
        set_bool(term->inputInfo(inputIndex)->properties.insert("state"), true);
        set_input_hidden(term, inputIndex, true);
    }
}

void set_step(Term* term, float step)
{
    term->setFloatProp("step", step);
}

float get_step(Term* term)
{
    return term->floatProp("step", 1.0);
}

void block_start_changes(Block* block)
{
    if (block->inProgress)
        return;

    block->inProgress = true;

    // For any minor block that is inProgress, make sure the parent block is also
    // marked inProgress.
    
    if (is_minor_block(block))
        block_start_changes(get_parent_block(block));
}

void block_finish_changes(Block* block)
{
    if (!block->inProgress) {
        // If nothing else, make sure bytecode is up to date.
        refresh_bytecode(block);
        return;
    }

    // Perform cleanup

    // Remove NULLs
    block->removeNulls();

    // Make sure nested minor blockes are finished.
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term->nestedContents != NULL && is_minor_block(term->nestedContents))
            block_finish_changes(term->nestedContents);
    }

    fix_forward_function_references(block);

    // Create an output_placeholder for state, if necessary.
    Term* openState = find_open_state_result(block, block->length());
    if (openState != NULL)
        append_state_output(block);

    update_exit_points(block);

    // Make sure the primary output is connected.
    if (is_minor_block(block)) {
        Term* output = get_output_placeholder(block, 0);

        // Don't mess with the primary if-block output.
        if (output != NULL && output->input(0) == NULL && !is_if_block(output->owningBlock)) {
            set_input(output, 0, find_last_non_comment_expression(block));
            respecialize_type(output);
        }
    }

    // Refresh for-loop zero block
    if (is_for_loop(block))
        for_loop_remake_zero_block(block);

    // Update block's state type
    block_update_state_type(block);

    dirty_bytecode(block);
    refresh_bytecode(block);

    block->inProgress = false;
    block->version++;
}

Term* find_user_with_function(Term* term, const char* funcName)
{
    for (int i=0; i < term->users.length(); i++) {
        Term* user = term->users[i];
        if (user->function->name == funcName)
            return user;
    }
    return NULL;
}
Term* apply_before(Term* existing, Term* function, int input)
{
    Block* block = existing->owningBlock;
    Term* newTerm = apply(block, function, TermList(existing->input(input)));
    block->move(newTerm, existing->index);
    set_input(existing, input, newTerm);
    return newTerm;
}
Term* apply_after(Term* existing, Term* function)
{
    Block* block = existing->owningBlock;

    // Grab a copy of users before we start messing with it
    TermList users = existing->users;

    Term* newTerm = apply(block, function, TermList(existing));
    block->move(newTerm, existing->index + 1);

    // Rewrite users to use the new term
    for (int i=0; i < users.length(); i++) {
        Term* user = users[i];
        remap_pointers_quick(user, existing, newTerm);
    }

    return newTerm;
}
void move_before(Term* movee, Term* position)
{
    ca_assert(movee->owningBlock == position->owningBlock);
    movee->owningBlock->move(movee, position->index);
}

void move_after(Term* movee, Term* position)
{
    Block* block = movee->owningBlock;
    int pos = position->index + 1;

    // Make sure the position is after any extra_output() terms
    while (pos < block->length()
            && block->get(pos) != NULL
            && block->get(pos)->function == FUNCS.extra_output)
        pos++;

    // If 'movee' is currently before 'position', then the desired index is one less
    if (movee->index < position->index)
        pos--;

    block->move(movee, pos);
}

void move_after_inputs(Term* term)
{
    Block* block = term->owningBlock;
    int inputCount = count_input_placeholders(block);
    block->move(term, inputCount);
}

bool term_belongs_at_block_end(Term* term)
{
    if (term == NULL)
        return false;

    if (term->function == FUNCS.output)
        return true;

    if (term->boolProp("final", false))
        return true;

    return false;
}

void move_before_outputs(Term* term)
{
    Block* block = term->owningBlock;

    // Walk backwards to find the target position
    int position = block->length();
    for (; position > 0; position--) {
        Term* preceding = block->get(position - 1);

        if (is_output_placeholder(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // an output term itself, then move the position back one more.
    if (!is_output_placeholder(term))
        position--;

    block->move(term, position);
}

void move_before_final_terms(Term* term)
{
    Block* block = term->owningBlock;

    // Walk backwards to find the target position
    int position = block->length();
    for (; position > 0; position--) {
        Term* preceding = block->get(position - 1);

        if (term_belongs_at_block_end(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // a final term itself, then move the position back one more.
    if (!term_belongs_at_block_end(term))
        position--;

    block->move(term, position);
}

void move_to_index(Term* term, int index)
{
    term->owningBlock->move(term, index);
}

void transfer_users(Term* from, Term* to)
{
    TermList users = from->users;
    for (int i=0; i < users.length(); i++) {
        if (users[i] == to)
            continue;
        remap_pointers_quick(users[i], from, to);
    }
}

void input_placeholders_to_list(Block* block, TermList* list)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            break;
        list->append(placeholder);
    }
}

void list_outer_pointers(Block* block, TermList* list)
{
    for (BlockInputIterator it(block); it.unfinished(); it.advance()) {
        if (it.currentInput()->owningBlock != block)
            list->appendUnique(it.currentInput());
    }
}

int find_input_index_for_pointer(Term* call, Term* input)
{
    for (int i=0; i < call->numInputs(); i++) {
        if (call->input(i) == input)
            return i;
    }
    return -1;
}

void check_to_add_primary_output_placeholder(Block* block)
{
    Term* output = get_output_placeholder(block, 0);

    if (output == NULL || is_state_input(output))
        prepend_output_placeholder(block, find_last_non_comment_expression(block));
}

void check_to_add_state_output_placeholder(Block* block)
{
    // No-op if a state output placeholder already exists
    if (find_state_output(block) != NULL)
        return;

    Term* result = find_open_state_result(block, block->length());

    // No-op if no state is being used
    if (result == NULL)
        return;

    Term* output = apply(block, FUNCS.output, TermList(result));
    output->setBoolProp("state", true);
}

Term* find_intermediate_result_for_output(Term* location, Term* output)
{
    // Check whether the output's connection is valid at this location
    Term* result = output->input(0);
    if (result != NULL
            && result->owningBlock == output->owningBlock
            && result->index < location->index)
        return result;

    // State output
    if (is_state_input(output))
        return find_open_state_result(location);

    // #return output
    if (result != NULL && result->name == "#return")
        return find_name_at(location, "#return");

    // Nearest with same name
    if (output->name != "")
        return find_name_at(location, output->name.c_str());

    return NULL;
}

void rewrite(Term* term, Term* function, TermList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);
    Type* outputType = function_get_output_type(function, 0);

    Function* attrs = as_function(term_value(function));

    if (attrs->specializeType != NULL)
        outputType = attrs->specializeType(term);

    change_declared_type(term, outputType);
}

void rewrite_as_value(Block* block, int index, Type* type)
{
    while (index > block->length())
        block->append(NULL);

    if (index >= block->length()) {
        create_value(block, type);
    } else {
        Term* term = block->get(index);

        change_function(term, FUNCS.value);
        change_declared_type(term, type);
        set_inputs(term, TermList());
    }
}

void remove_term(Term* term)
{
    assert_valid_term(term);

    int index = term->index;
    Block* block = term->owningBlock;

    erase_term(term);

    for (int i=index; i < block->_terms.length()-1; i++) {
        block->_terms.setAt(i, block->_terms[i+1]);
        if (block->_terms[i] != NULL)
            block->_terms[i]->index = i;
    }
    block->_terms.resize(block->_terms.length()-1);
}

void remap_pointers_quick(Term* term, Term* old, Term* newTerm)
{
    for (int i=0; i < term->numInputs(); i++)
        if (term->input(i) == old)
            set_input(term, i, newTerm);
}

void remap_pointers_quick(Block* block, Term* old, Term* newTerm)
{
    for (int i=0; i < block->length(); i++)
        remap_pointers_quick(block->get(i), old, newTerm);
}

void remap_pointers(Term* term, TermMap const& map)
{
    assert_valid_term(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    ca_assert(!map.contains(NULL));

    for (int i=0; i < term->numInputs(); i++)
        set_input(term, i, map.getRemapped(term->input(i)));

    term->function = map.getRemapped(term->function);

    // TODO, call changeType if our type is changed
    
    Type::RemapPointers remapPointers = term->type->remapPointers;

    // Remap on value
    if ((term_value(term)->value_data.ptr != NULL)
            && term->type != NULL
            && (remapPointers)) {

        remapPointers(term, map);
    }

    // This code once called remap on term->properties

    // Remap inside nestedContents
    if (has_nested_contents(term))
        nested_contents(term)->remapPointers(map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_valid_term(term);
    assert_valid_term(original);
    ca_assert(original != NULL);

    TermMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void remap_pointers(Block* block, Term* original, Term* replacement)
{
    TermMap map;
    map[original] = replacement;

    for (int i=0; i < block->length(); i++) {
        if (block->get(i) == NULL) continue;
        remap_pointers(block->get(i), map);
    }
}

bool term_is_nested_in_block(Term* term, Block* block)
{
    while (term != NULL) {
        if (term->owningBlock == block)
            return true;

        term = get_parent_term(term);
    }

    return false;
}

void create_inputs_for_outer_references(Term* term)
{
    Block* block = nested_contents(term);
    TermMap outerToInnerMap;

    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* innerTerm = *it;
        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (!term_is_nested_in_block(input, block)) {
                // This is an outer reference

                // Check if we've already created a placeholder for this one
                Term* existingPlaceholder = outerToInnerMap[input];

                if (existingPlaceholder != NULL) {
                    remap_pointers_quick(innerTerm, input, existingPlaceholder);

                } else {
                    // Need to create a new placeholder
                    int placeholderIndex = term->numInputs();
                    Term* placeholder = apply(block, FUNCS.input, TermList(), input->nameSymbol);
                    change_declared_type(placeholder, input->type);
                    block->move(placeholder, placeholderIndex);
                    set_input(term, placeholderIndex, placeholder);
                    remap_pointers_quick(innerTerm, input, placeholder);
                }
            }
        }
    }
}

} // namespace circa
