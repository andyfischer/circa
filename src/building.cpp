// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "building.h"
#include "evaluation.h"
#include "function.h"
#include "heap_debugging.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "locals.h"
#include "loops.h"
#include "names.h"
#include "parser.h"
#include "names.h"
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

Term* apply(Branch* branch, Term* function, TermList const& inputs, std::string const& name)
{
    branch_start_changes(branch);

    // If function is NULL, use 'unknown_function' instead.
    if (function == NULL)
        function = FUNCS.unknown_function;

    // If 'function' is actually a type, create a call to cast().
    if (function != NULL && is_type(function)) {
        Term* term = apply(branch, FUNCS.cast, inputs);
        change_declared_type(term, as_type(term_value(function)));
        return term;
    }

    // Figure out the term position; it should be placed before any output() terms.
    // (unless it's an output term itself).
    int position = branch->length();
    if (function != FUNCS.output) {
        while (position > 0) {
            Term* preceding = branch->get(position - 1);
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
    Term* term = branch->appendNew();
    INCREMENT_STAT(TermsCreated);

    on_term_created(term);

    // Position the term before any output_placeholder terms.
    branch->move(term, position);

    if (name != "")
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
    branch_start_changes(term->owningBranch);

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
        find_or_create_state_container(term->owningBranch);

    dirty_bytecode(term->owningBranch);
}

void change_declared_type(Term *term, Type *newType)
{
    // Don't allow 'null' to be used as a declared type (use 'any' instead)
    if (newType == &NULL_T)
        newType = &ANY_T;

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
    if (SHUTTING_DOWN)
        return;

    Type* outputType = derive_specialized_output_type(term->function, term);
    if (outputType != term->type)
        change_declared_type(term, outputType);
}

void rename(Term* term, std::string const& name)
{
    if (term->name == name)
        return;

    if (term->owningBranch != NULL) {
        if (term->name != "") {
            term->owningBranch->names.remove(term->name);
            term->name = "";
        }
        term->owningBranch->bindName(term, name);
    }

    std::string prevName = term->name;
    term->name = name;
    update_unique_name(term);

    on_term_name_changed(term, prevName.c_str(), name.c_str());
}

Term* create_duplicate(Branch* branch, Term* original, std::string const& name)
{
    ca_assert(original != NULL);

    TermList inputs;
    original->inputsToList(inputs);

    Term* term = apply(branch, original->function, inputs, name);
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

Term* apply(Branch* branch, std::string const& functionName, TermList const& inputs, std::string const& name)
{
    Term* function = find_name(branch, functionName.c_str());
    if (function == NULL)
        internal_error("function not found: "+functionName);

    Term* result = apply(branch, function, inputs, name);
    result->setStringProp("syntax:functionName", functionName.c_str());
    return result;
}

Term* create_value(Branch* branch, Type* type, std::string const& name)
{
    // This function is safe to call while bootstrapping.
    ca_assert(type != NULL);

    Term *term = apply(branch, FUNCS.value, TermList(), name);

    change_declared_type(term, type);
    create(type, term_value(term));

    if (type == &TYPE_T) {
        as_type(term)->name = name_from_string(name.c_str());
        as_type(term)->declaringTerm = term;
    }

    return term;
}

Term* create_value(Branch* branch, std::string const& typeName, std::string const& name)
{
    Term* type = NULL;

    type = find_name(branch, typeName.c_str());

    if (type == NULL)
        internal_error("Couldn't find type: "+typeName);

    return create_value(branch, as_type(term_value(type)), name);
}

Term* create_value(Branch* branch, caValue* initialValue, std::string const& name)
{
    Term* term = create_value(branch, initialValue->value_type, name);
    copy(initialValue, term_value(term));
    return term;
}

Term* create_string(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, &STRING_T, name);
    set_string(term_value(term), s);
    return term;
}

Term* create_int(Branch* branch, int i, std::string const& name)
{
    Term* term = create_value(branch, &INT_T, name);
    set_int(term_value(term), i);
    return term;
}

Term* create_float(Branch* branch, float f, std::string const& name)
{
    Term* term = create_value(branch, &FLOAT_T, name);
    set_float(term_value(term), f);
    return term;
}

Term* create_bool(Branch* branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, &BOOL_T, name);
    set_bool(term_value(term), b);
    return term;
}

Term* create_void(Branch* branch, std::string const& name)
{
    return create_value(branch, &VOID_T, name);
}

Term* create_list(Branch* branch, std::string const& name)
{
    Term* term = create_value(branch, &LIST_T, name);
    return term;
}

Branch* create_branch(Branch* owner, std::string const& name)
{
    return apply(owner, FUNCS.branch, TermList(), name)->contents();
}

Branch* create_namespace(Branch* branch, std::string const& name)
{
    return apply(branch, FUNCS.namespace_func, TermList(), name)->contents();
}
Branch* create_branch_unevaluated(Branch* owner, const char* name)
{
    return nested_contents(apply(owner, FUNCS.branch_unevaluated, TermList(), name));
}

Term* create_type(Branch* branch, std::string name)
{
    Term* term = create_value(branch, &TYPE_T);

    if (name != "") {
        as_type(term_value(term))->name = name_from_string(name.c_str());
        rename(term, name);
    }

    return term;
}

Term* create_type_value(Branch* branch, Type* value, std::string const& name)
{
    Term* term = create_value(branch, &TYPE_T, name);
    set_type(term_value(term), value);

    if (value->declaringTerm == NULL)
        value->declaringTerm = term;
        
    return term;
}

Term* create_symbol_value(Branch* branch, int value, std::string const& name)
{
    Term* term = create_value(branch, &NAME_T, name);
    set_name(term_value(term), value);
    return term;
}

Term* duplicate_value(Branch* branch, Term* term)
{
    Term* dup = create_value(branch, term->type);
    copy(term_value(term), term_value(dup));
    return dup;
}

Term* append_input_placeholder(Branch* branch)
{
    int count = count_input_placeholders(branch);
    Term* term = apply(branch, FUNCS.input, TermList());
    branch->move(term, count);
    return term;
}
Term* append_output_placeholder(Branch* branch, Term* result)
{
    int count = count_output_placeholders(branch);
    Term* term = apply(branch, FUNCS.output, TermList(result));
    branch->move(term, branch->length() - count - 1);
    return term;
}
Term* prepend_output_placeholder(Branch* branch, Term* result)
{
    return apply(branch, FUNCS.output, TermList(result));
}

Branch* term_get_function_details(Term* call)
{
    // TODO: Shouldn't need to special case these functions.
    if (call->function == FUNCS.if_block
        || call->function == FUNCS.for_func
        || call->function == FUNCS.include_func)
        return nested_contents(call);

    return function_get_contents(as_function(term_value(call->function)));
}

void update_extra_outputs(Term* term)
{
    Branch* branch = term->owningBranch;
    Branch* function = term_get_function_details(term);

    bool needToUpdatePackState = false;

    for (int index=1; ; index++) {
        Term* placeholder = get_output_placeholder(function, index);
        if (placeholder == NULL)
            break;

        const char* name = "";

        // Find the associated input placeholder (if any).
        Term* associatedInput = NULL;

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
                name = input->name.c_str();

        } else {
            name = placeholder->name.c_str();
        }

        Term* extra_output = NULL;

        // Check if this extra_output() already exists
        Term* existingSlot = branch->getSafe(term->index + index);
        if (existingSlot != NULL && existingSlot->function == FUNCS.extra_output)
            extra_output = existingSlot;
        
        if (extra_output == NULL) {
            extra_output = apply(branch, FUNCS.extra_output, TermList(term), name);
            move_to_index(extra_output, term->index + index);

            if (is_state_input(placeholder))
                needToUpdatePackState = true;
        }

        change_declared_type(extra_output, placeholder->type);

        if (is_state_input(placeholder))
            extra_output->setBoolProp("state", true);
    }

    if (needToUpdatePackState)
        branch_update_existing_pack_state_calls(branch);
}

Term* get_output_term(Term* term, int index)
{
    if (index == 0)
        return term;
    else
        return get_extra_output(term, index - 1);
}

Term* get_extra_output(Term* term, int index)
{
    Term* position = term->owningBranch->getSafe(term->index + index + 1);
    if (position != NULL && position->function == FUNCS.extra_output)
        return position;
    return NULL;
}

Term* find_extra_output_for_state(Term* term)
{
    for (int i=0;; i++) {
        Term* extra_output = get_extra_output(term, i);
        if (extra_output == NULL)
            break;

        if (extra_output->boolProp("state", false))
            return extra_output;
    }
    return NULL;
}

Term* term_get_input_placeholder(Term* call, int index)
{
    if (!is_function(call->function))
        return NULL;

    Branch* contents = term_get_function_details(call);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->get(index);
    if (term->function != FUNCS.input)
        return NULL;
    return term;
}

int term_count_input_placeholders(Term* term)
{
    int result = 0;
    while (term_get_input_placeholder(term, result) != NULL)
        result++;
    return result;
}

Term* term_get_output_placeholder(Term* call, int index)
{
    if (!is_function(call->function))
        return NULL;

    Branch* contents = term_get_function_details(call);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->getFromEnd(index);
    if (term->function != FUNCS.output)
        return NULL;
    return term;
}
bool term_has_variable_args(Term* term)
{
    return has_variable_args(term_get_function_details(term));
}

Term* find_open_state_result(Branch* branch, int position)
{
    for (int i = position - 1; i >= 0; i--) {
        Term* term = branch->get(i);
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
    return find_open_state_result(location->owningBranch, location->index);
}

void check_to_insert_implicit_inputs(Term* term)
{
    if (!is_function(term->function))
        return;

    Term* stateInput = find_active_state_container(term_get_function_details(term));

    if (stateInput != NULL && !term_is_state_input(term, stateInput->index)) {

        int inputIndex = stateInput->index;

        Term* container = find_or_create_state_container(term->owningBranch);

        // Add a unpack_state() call
        Term* unpack = apply(term->owningBranch, FUNCS.unpack_state,
            TermList(container, term));
        hide_from_source(unpack);
        term->owningBranch->move(unpack, term->index);

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

void branch_start_changes(Branch* branch)
{
    if (branch->inProgress)
        return;

    branch->inProgress = true;

    // For any minor branch that is inProgress, make sure the parent branch is also
    // marked inProgress.
    
    if (is_minor_branch(branch))
        branch_start_changes(get_parent_branch(branch));
}

void branch_finish_changes(Branch* branch)
{
    if (!branch->inProgress) {
        // If nothing else, make sure bytecode is up to date.
        refresh_bytecode(branch);
        return;
    }

    // Perform cleanup

    // Remove NULLs
    branch->removeNulls();

    // Make sure nested minor branches are finished.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

        if (term->nestedContents != NULL && is_minor_branch(term->nestedContents))
            branch_finish_changes(term->nestedContents);
    }

    fix_forward_function_references(branch);

    // Create an output_placeholder for state, if necessary.
    Term* openState = find_open_state_result(branch, branch->length());
    if (openState != NULL)
        append_state_output(branch);

    update_exit_points(branch);

    // Make sure primary output is connected
    if (is_minor_branch(branch)) {
        Term* output = get_output_placeholder(branch, 0);
        if (output != NULL && output->input(0) == NULL) {
            set_input(output, 0, find_last_non_comment_expression(branch));
            respecialize_type(output);
        }
    }

    // Refresh for-loop zero branch
    if (is_for_loop(branch))
        for_loop_remake_zero_branch(branch);

    // Update branch's state type
    branch_update_state_type(branch);

    dirty_bytecode(branch);
    refresh_bytecode(branch);

    branch->inProgress = false;
    branch->version++;
}

bool term_is_state_input(Term* term, int index)
{
    if (index >= term->numInputs())
        return false;
    caValue* prop = term->inputInfo(index)->properties.get("state");
    if (prop == NULL)
        return false;
    return as_bool(prop);
}

Term* find_state_input(Branch* branch)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            return NULL;
        if (is_state_input(placeholder))
            return placeholder;
    }
}

bool has_state_input(Branch* branch)
{
    return find_state_input(branch) != NULL;
}

Term* find_state_output(Branch* branch)
{
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(branch, i);
        if (placeholder == NULL)
            return NULL;
        if (is_state_output(placeholder))
            return placeholder;
    }
}
bool has_state_output(Branch* branch)
{
    return find_state_output(branch) != NULL;
}
Term* append_state_input(Branch* branch)
{
    // Make sure that a state input doesn't already exist
    Term* existing = find_state_input(branch);
    if (existing != NULL)
        return existing;

    int inputCount = count_input_placeholders(branch);

    Term* term = apply(branch, FUNCS.input, TermList());
    branch->move(term, inputCount);
    term->setBoolProp("state", true);
    term->setBoolProp("hiddenInput", true);
    term->setBoolProp("output", true);

    on_branch_inputs_changed(branch);

    return term;
}

Term* append_state_output(Branch* branch)
{
    // Make sure that a state input doesn't already exist
    Term* existing = find_state_output(branch);
    if (existing != NULL)
        return existing;

    Term* term = append_output_placeholder(branch, 
        find_open_state_result(branch, branch->length()));
    term->setBoolProp("state", true);
    hide_from_source(term);
    return term;
}
bool is_state_input(Term* placeholder)
{
    return placeholder->boolProp("state", false);
}
bool is_state_output(Term* placeholder)
{
    return placeholder->boolProp("state", false);
}

Term* find_parent_term_in_branch(Term* term, Branch* branch)
{
    while (true) {
        if (term == NULL)
            return NULL;

        if (term->owningBranch == branch)
            return term;

        term = get_parent_term(term);
    }
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
    Branch* branch = existing->owningBranch;
    Term* newTerm = apply(branch, function, TermList(existing->input(input)));
    branch->move(newTerm, existing->index);
    set_input(existing, input, newTerm);
    return newTerm;
}
Term* apply_after(Term* existing, Term* function)
{
    Branch* branch = existing->owningBranch;

    // Grab a copy of users before we start messing with it
    TermList users = existing->users;

    Term* newTerm = apply(branch, function, TermList(existing));
    branch->move(newTerm, existing->index + 1);

    // Rewrite users to use the new term
    for (int i=0; i < users.length(); i++) {
        Term* user = users[i];
        remap_pointers_quick(user, existing, newTerm);
    }

    return newTerm;
}
void move_before(Term* movee, Term* position)
{
    ca_assert(movee->owningBranch == position->owningBranch);
    movee->owningBranch->move(movee, position->index);
}

void move_after(Term* movee, Term* position)
{
    Branch* branch = movee->owningBranch;
    int pos = position->index + 1;

    // Make sure the position is after any extra_output() terms
    while (pos < branch->length()
            && branch->get(pos) != NULL
            && branch->get(pos)->function == FUNCS.extra_output)
        pos++;

    // If 'movee' is currently before 'position', then the desired index is one less
    if (movee->index < position->index)
        pos--;

    branch->move(movee, pos);
}

void move_after_inputs(Term* term)
{
    Branch* branch = term->owningBranch;
    int inputCount = count_input_placeholders(branch);
    branch->move(term, inputCount);
}

bool term_belongs_at_branch_end(Term* term)
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
    Branch* branch = term->owningBranch;

    // Walk backwards to find the target position
    int position = branch->length();
    for (; position > 0; position--) {
        Term* preceding = branch->get(position - 1);

        if (is_output_placeholder(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // an output term itself, then move the position back one more.
    if (!is_output_placeholder(term))
        position--;

    branch->move(term, position);
}

void move_before_final_terms(Term* term)
{
    Branch* branch = term->owningBranch;

    // Walk backwards to find the target position
    int position = branch->length();
    for (; position > 0; position--) {
        Term* preceding = branch->get(position - 1);

        if (term_belongs_at_branch_end(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // a final term itself, then move the position back one more.
    if (!term_belongs_at_branch_end(term))
        position--;

    branch->move(term, position);
}

void move_to_index(Term* term, int index)
{
    term->owningBranch->move(term, index);
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

void input_placeholders_to_list(Branch* branch, TermList* list)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            break;
        list->append(placeholder);
    }
}

void list_outer_pointers(Branch* branch, TermList* list)
{
    for (BranchInputIterator it(branch); it.unfinished(); it.advance()) {
        if (it.currentInput()->owningBranch != branch)
            list->appendUnique(it.currentInput());
    }
}

void expand_variadic_inputs_for_call(Branch* branch, Term* call)
{
    Term* input0 = get_input_placeholder(branch, 0);
    if (input0 == NULL || !input0->boolProp("multiple", false))
        return;

    // Add extra input_placeholder term
    int inputCount = call->numInputs();
    input0->removeProperty("multiple");

    for (int i=1; i < inputCount; i++) {
        append_input_placeholder(branch);
    }

    // Modify calls to use these placeholders
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            if (term->input(inputIndex) == input0) {

                for (int extraInput=1; extraInput < inputCount; extraInput++) {
                    set_input(term, extraInput, get_input_placeholder(branch, extraInput));
                }
            }
        }
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

void check_to_add_primary_output_placeholder(Branch* branch)
{
    Term* output = get_output_placeholder(branch, 0);

    if (output == NULL || is_state_input(output))
        prepend_output_placeholder(branch, find_last_non_comment_expression(branch));
}

void check_to_add_state_output_placeholder(Branch* branch)
{
    // No-op if a state output placeholder already exists
    if (find_state_output(branch) != NULL)
        return;

    Term* result = find_open_state_result(branch, branch->length());

    // No-op if no state is being used
    if (result == NULL)
        return;

    Term* output = apply(branch, FUNCS.output, TermList(result));
    output->setBoolProp("state", true);
}

Term* find_intermediate_result_for_output(Term* location, Term* output)
{
    // Check whether the output's connection is valid at this location
    Term* result = output->input(0);
    if (result != NULL
            && result->owningBranch == output->owningBranch
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

void rewrite_as_value(Branch* branch, int index, Type* type)
{
    while (index > branch->length())
        branch->append(NULL);

    if (index >= branch->length()) {
        create_value(branch, type);
    } else {
        Term* term = branch->get(index);

        change_function(term, FUNCS.value);
        change_declared_type(term, type);
        set_inputs(term, TermList());
    }
}

void remove_term(Term* term)
{
    assert_valid_term(term);

    int index = term->index;
    Branch* branch = term->owningBranch;

    erase_term(term);

    for (int i=index; i < branch->_terms.length()-1; i++) {
        branch->_terms.setAt(i, branch->_terms[i+1]);
        if (branch->_terms[i] != NULL)
            branch->_terms[i]->index = i;
    }
    branch->_terms.resize(branch->_terms.length()-1);
}

void remap_pointers_quick(Term* term, Term* old, Term* newTerm)
{
    for (int i=0; i < term->numInputs(); i++)
        if (term->input(i) == old)
            set_input(term, i, newTerm);
}

void remap_pointers_quick(Branch* branch, Term* old, Term* newTerm)
{
    for (int i=0; i < branch->length(); i++)
        remap_pointers_quick(branch->get(i), old, newTerm);
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

void remap_pointers(Branch* branch, Term* original, Term* replacement)
{
    TermMap map;
    map[original] = replacement;

    for (int i=0; i < branch->length(); i++) {
        if (branch->get(i) == NULL) continue;
        remap_pointers(branch->get(i), map);
    }
}

Term* write_selector_for_accessor_expression(Branch* branch, Term* accessor, Term** headPtr)
{
    TermList elements;

    while (true) {
        if (headPtr != NULL)
            *headPtr = accessor;

        // Stop if we find a term with a name.
        if (accessor->name != "")
            break;

        if (accessor->function == FUNCS.get_index) {
            elements.prepend(accessor->input(1));
            accessor = accessor->input(0);
        } else if (accessor->function == FUNCS.get_field) {
            elements.prepend(accessor->input(1));
            accessor = accessor->input(0);
        } else if (accessor->function == FUNCS.dynamic_method) {
            Term* fieldName = create_string(branch,
                    accessor->stringProp("syntax:functionName", ""));
            elements.prepend(fieldName);
            accessor = accessor->input(0);
        } else {
            break;
        }
    }

    if (elements.length() == 0)
        return NULL;

    return apply(branch, FUNCS.selector, elements);
}

Term* write_set_selector_result(Branch* branch, Term* accessorExpr, Term* result)
{
    Term* head = NULL;
    Term* selector = write_selector_for_accessor_expression(branch, accessorExpr, &head);
    if (selector != NULL) {
        Term* set = apply(branch, FUNCS.set_with_selector, TermList(head, selector, result));
        change_declared_type(set, declared_type(head));
        rename(set, head->name);
        return set;
    } else {
        rename(result, accessorExpr->name);
        result->setBoolProp("syntax:implicitName", true);
        return result;
    }
}

bool term_is_nested_in_branch(Term* term, Branch* branch)
{
    while (term != NULL) {
        if (term->owningBranch == branch)
            return true;

        term = get_parent_term(term);
    }

    return false;
}

void create_inputs_for_outer_references(Term* term)
{
    Branch* branch = nested_contents(term);
    TermMap outerToInnerMap;

    for (BranchIterator it(branch); it.unfinished(); it.advance()) {
        Term* innerTerm = *it;
        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (!term_is_nested_in_branch(input, branch)) {
                // This is an outer reference

                // Check if we've already created a placeholder for this one
                Term* existingPlaceholder = outerToInnerMap[input];

                if (existingPlaceholder != NULL) {
                    remap_pointers_quick(innerTerm, input, existingPlaceholder);

                } else {
                    // Need to create a new placeholder
                    int placeholderIndex = term->numInputs();
                    Term* placeholder = apply(branch, FUNCS.input, TermList(), input->name);
                    change_declared_type(placeholder, input->type);
                    branch->move(placeholder, placeholderIndex);
                    set_input(term, placeholderIndex, placeholder);
                    remap_pointers_quick(innerTerm, input, placeholder);
                }
            }
        }
    }
}

} // namespace circa
