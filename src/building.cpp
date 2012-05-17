// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "building.h"
#include "function.h"
#include "heap_debugging.h"
#include "kernel.h"
#include "introspection.h"
#include "list.h"
#include "locals.h"
#include "names.h"
#include "parser.h"
#include "names.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

namespace circa {

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
                    && preceding->boolPropOptional("final", false)) {
                position--;
                continue;
            }

            break;
        }
    }

    // Create the term
    Term* term = branch->appendNew();

    // Position the term before any output_placeholder terms.
    branch->move(term, position);

    if (name != "")
        rename(term, name);

    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);

    // change_function will also update the declared type.
    change_function(term, function);

    update_unique_name(term);
    on_inputs_changed(term);
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

    mark_inputs_changed(term);
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

    mark_inputs_changed(term);
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

bool is_actually_using(Term* user, Term* usee)
{
    for (int i=0; i < user->numDependencies(); i++)
        if (user->dependency(i) == usee)
            return true;

    return false;
}

void append_user(Term* user, Term* usee)
{
    if (usee != NULL && user != NULL)
        usee->users.appendUnique(user);
}

void possibly_prune_user_list(Term* user, Term* usee)
{
    if (usee != NULL && !is_actually_using(user, usee))
        usee->users.remove(user);
}

void remove_from_any_user_lists(Term* term)
{
    for (int i=0; i < term->numDependencies(); i++) {
        Term* usee = term->dependency(i);
        assert_valid_term(usee);
        if (usee == NULL)
            continue;
        usee->users.remove(term);
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
        find_or_create_state_input(term->owningBranch);
}

void unsafe_change_type(Term *term, Type *type)
{
    ca_assert(type != NULL);

    term->type = type;
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

    // TODO: Don't call create() here
    create(newType, term_value(term));

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

void specialize_type(Term *term, Type *type)
{
    if (term->type == type)
        return;

    ca_assert(term->type == &ANY_T);

    change_declared_type(term, type);
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

    term->name = name;
    update_unique_name(term);
}

Term* create_duplicate(Branch* branch, Term* original, std::string const& name, bool copyBranches)
{
    ca_assert(original != NULL);

    TermList inputs;
    original->inputsToList(inputs);

    Term* term = apply(branch, original->function, inputs, name);
    change_declared_type(term, original->type);
    //create(original->value_type, term_value(term));

    copy(term_value(original), term_value(term));

    if (copyBranches)
        duplicate_branch(nested_contents(original), nested_contents(term));

    term->sourceLoc = original->sourceLoc;
    copy(&original->properties, &term->properties);

    // Special case for certain types, update declaringType
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
    result->setStringProp("syntax:functionName", functionName);
    return result;
}

Term* create_value(Branch* branch, Type* type, std::string const& name)
{
    // This function is safe to call while bootstrapping.
    ca_assert(type != NULL);

    Term *term = apply(branch, FUNCS.value, TermList(), name);

    change_declared_type(term, type);
    create(type, term_value(term));

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

Term* procure_value(Branch* branch, Type* type, std::string const& name)
{
    Term* existing = branch->get(name);
    if (existing == NULL)
        existing = create_value(branch, type, name);
    else
        change_declared_type(existing, type);
    return existing;
}

Term* procure_int(Branch* branch, std::string const& name)
{
    return procure_value(branch, &INT_T, name);
}

Term* procure_float(Branch* branch, std::string const& name)
{
    return procure_value(branch, &FLOAT_T, name);
}

Term* procure_bool(Branch* branch, std::string const& name)
{
    return procure_value(branch, &BOOL_T, name);
}

Term* get_input_placeholder(Branch* branch, int index)
{
    if (index >= branch->length())
        return NULL;
    Term* term = branch->get(index);
    if (term == NULL || term->function != FUNCS.input)
        return NULL;
    return term;
}

Term* get_output_placeholder(Branch* branch, int index)
{
    if (index >= branch->length())
        return NULL;
    Term* term = branch->getFromEnd(index);
    if (term == NULL || term->function != FUNCS.output)
        return NULL;
    return term;
}

int count_input_placeholders(Branch* branch)
{
    int result = 0;
    while (get_input_placeholder(branch, result) != NULL)
        result++;
    return result;
}
int count_output_placeholders(Branch* branch)
{
    int result = 0;
    while (get_output_placeholder(branch, result) != NULL)
        result++;
    return result;
}
bool is_input_placeholder(Term* term)
{
    return term->function == FUNCS.input;
}
bool is_output_placeholder(Term* term)
{
    return term->function == FUNCS.output;
}
bool has_variable_args(Branch* branch)
{
    Term* input0 = get_input_placeholder(branch, 0);
    if (input0 == NULL)
        return false;
    return input0->boolPropOptional("multiple", false);
}    
Term* find_output_placeholder_with_name(Branch* branch, const char* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(branch, i);
        if (placeholder == NULL)
            return NULL;
        if (placeholder->name == name)
            return placeholder;
    }
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

    for (int index=1; ; index++) {
        Term* placeholder = term_get_output_placeholder(term, index);
        if (placeholder == NULL)
            break;

        const char* name = "";

        int rebindsInput = placeholder->intPropOptional("rebindsInput", -1);
        if (rebindsInput != -1 && rebindsInput < term->numInputs()) {
            name = term->input(rebindsInput)->name.c_str();
        } else {
            name = placeholder->name.c_str();
        }

        Term* extra_output = NULL;

        // Check if this extra_output() already exists
        Term* existingSlot = branch->getSafe(term->index + index);
        if (existingSlot != NULL && existingSlot->function == EXTRA_OUTPUT_FUNC)
            extra_output = existingSlot;
        
        if (extra_output == NULL) {
            extra_output = apply(term->owningBranch, EXTRA_OUTPUT_FUNC, TermList(term), name);
            move_to_index(extra_output, term->index + index);
        }

        change_declared_type(extra_output, placeholder->type);

        if (function_is_state_input(placeholder))
            extra_output->setBoolProp("state", true);
    }
}

Term* get_extra_output(Term* term, int index)
{
    Term* position = term->owningBranch->getSafe(term->index + index + 1);
    if (position != NULL && position->function == EXTRA_OUTPUT_FUNC)
        return position;
    return NULL;
}

Term* find_extra_output_for_state(Term* term)
{
    for (int i=0;; i++) {
        Term* extra_output = get_extra_output(term, i);
        if (extra_output == NULL)
            break;

        if (extra_output->boolPropOptional("state", false))
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
        if (term->function == FUNCS.input && function_is_state_input(term))
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

    Term* stateInput = find_state_input(term_get_function_details(term));

    if (stateInput != NULL && !term_is_state_input(term, stateInput->index)) {

        int inputIndex = stateInput->index;

        Term* container = find_or_create_state_input(term->owningBranch);

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
    return term->floatPropOptional("step", 1.0);
}
bool is_lazy_call(Term* term)
{
    return term->flags & TERM_FLAG_LAZY;
}
void set_lazy_call(Term* term, bool value)
{
    term->setBoolProp("lazy", value);
    term->flags = (term->flags & ~TERM_FLAG_LAZY) + (value ? TERM_FLAG_LAZY : 0);
}

void create_rebind_branch(Branch* rebinds, Branch* source, Term* rebindCondition, bool outsidePositive)
{
    clear_branch(rebinds);

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(source, reboundNames);

    Branch* outerScope = source->owningTerm->owningBranch;
    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Term* outerVersion = find_name(outerScope, name.c_str());
        Term* innerVersion = source->get(name);

        Term* pos = outsidePositive ? outerVersion : innerVersion;
        Term* neg = outsidePositive ? innerVersion : outerVersion ;
        apply(rebinds, FUNCS.cond, TermList(rebindCondition, pos, neg), name);
    }
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
    if (!branch->inProgress)
        return;

    // Perform cleanup

    // Remove NULLs
    branch->removeNulls();

    // Finish nested minor branches
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);

        if (term->nestedContents != NULL && is_minor_branch(term->nestedContents))
            branch_finish_changes(term->nestedContents);
    }

    finish_update_cascade(branch);

    fix_forward_function_references(branch);

    // Create an output_placeholder for state, if necessary.
    Term* openState = find_open_state_result(branch, branch->length());
    if (openState != NULL)
        insert_state_output(branch);

    update_exit_points(branch);

    // Make sure primary output is connected
    if (is_minor_branch(branch)) {
        Term* output = get_output_placeholder(branch, 0);
        if (output != NULL && output->input(0) == NULL) {
            set_input(output, 0, find_last_non_comment_expression(branch));
        }
    }

    // Update branch's state type
    branch_update_state_type(branch);

    branch->inProgress = false;
    branch->version++;
}

Term* find_last_non_comment_expression(Branch* branch)
{
    for (int i = branch->length() - 1; i >= 0; i--) {
        if (branch->get(i) == NULL)
            continue;
        if (branch->get(i)->function == FUNCS.output)
            continue;
        if (branch->get(i)->function == FUNCS.input)
            continue;
        if (branch->get(i)->name == "#outer_rebinds")
            continue;
        if (branch->get(i)->function != FUNCS.comment)
            return branch->get(i);
    }
    return NULL;
}

Term* find_term_with_function(Branch* branch, Term* func)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->getFromEnd(i);
        if (term == NULL)
            continue;
        if (term->function == func)
            return term;
    }
    return NULL;
}

Term* find_input_with_function(Term* target, Term* func)
{
    for (int i=0; i < target->numInputs(); i++) {
        Term* input = target->input(i);
        if (input == NULL) continue;
        if (input->function == func)
            return input;
    }
    return NULL;
}

Term* find_user_with_function(Term* target, Term* func)
{
    for (int i=0; i < target->users.length(); i++)
        if (target->users[i]->function == func)
            return target->users[i];
    return NULL;
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
    return NULL;
}

bool has_state_input(Branch* branch)
{
    return find_state_input(branch) != NULL;
}

Term* find_state_output(Branch* branch)
{
    for (int i=branch->length() - 1; i >= 0; i--) {
        Term* placeholder = branch->get(i);
        if (placeholder->function != FUNCS.output)
            break;
        if (function_is_state_input(placeholder))
            return placeholder;
    }
    return NULL;
}
bool has_state_output(Branch* branch)
{
    return find_state_output(branch) != NULL;
}
Term* append_state_input(Branch* branch)
{
    // Make sure that a state input doesn't already exist
    Term* term = find_state_input(branch);
    if (term != NULL)
        return term;

    int inputCount = count_input_placeholders(branch);

    term = apply(branch, FUNCS.input, TermList());
    branch->move(term, inputCount);
    term->setBoolProp("state", true);
    term->setBoolProp("hiddenInput", true);
    term->setBoolProp("output", true);
    return term;
}
Term* insert_state_output(Branch* branch)
{
    // Make sure that a state input doesn't already exist
    Term* term = find_state_output(branch);
    Term* openStateResult = find_open_state_result(branch, branch->length());

    if (term != NULL) {
        set_input(term, 0, openStateResult);
        return term;
    }
    
    term = apply(branch, FUNCS.output, TermList(openStateResult));
    term->setBoolProp("state", true);
    hide_from_source(term);
    return term;
}
Term* append_state_output(Branch* branch)
{
    Term* term = append_output_placeholder(branch, 
        find_open_state_result(branch, branch->length()));
    term->setBoolProp("state", true);
    hide_from_source(term);
    return term;
}
bool is_state_input(Term* placeholder)
{
    return placeholder->boolPropOptional("state", false);
}
bool is_state_output(Term* placeholder)
{
    return placeholder->boolPropOptional("state", false);
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
    while (branch->get(pos) != NULL && branch->get(pos)->function == EXTRA_OUTPUT_FUNC)
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

    if (term->boolPropOptional("final", false))
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
    if (input0 == NULL || !input0->boolPropOptional("multiple", false))
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

    if (is_list(&branch->pendingUpdates))
        list_remove_index(&branch->pendingUpdates, index);
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


// For a given 'get' expression, such as:
//   result = get_field(container, field)
//
// This function will create a term that replaces the 'get' result with 'desiredValue'.
// So for the above example, we would generate:
//   set_field(container, field, desiredValue)
//
// If the term is not a recognized getter then we return NULL

Term* write_setter_from_getter(Branch* branch, Term* term, Term* desiredValue)
{
    if (term->function == FUNCS.get_index) {
        return apply(branch, FUNCS.set_index, TermList(term->input(0), term->input(1), desiredValue));
    } else if (term->function == FUNCS.get_field) {
        return apply(branch, FUNCS.set_field, TermList(term->input(0), term->input(1), desiredValue));
    } else if (term->function == FUNCS.dynamic_method) {
        Term* fieldName = create_string(branch, term->stringProp("syntax:functionName"));
        return apply(branch, FUNCS.set_field, TermList(term->input(0), fieldName, desiredValue));
    } else {
        return NULL;
    }

}

Term* write_setter_chain_from_getter_chain(Branch* branch, Term* getterRoot, Term* desired)
{
    /*
     * Consider the following expression:
     *
     * a[i0][i1][i2] = y
     *
     * The terms would look like this:
     * 
     * a = (some compound type)
     * i0 = (an integer index)
     * i1 = (an integer index)
     * i2 = (an integer index)
     * a_0 = get_index(a, i0)
     * a_1 = get_index(a_0, i1)
     * a_2 = get_index(a_1, i2)
     * assign(a_2, y)
     *
     * (Some names are made up for clarity, the only terms that actually have names
     * are "a" and "y")
     *
     * In order to do the assignment in a pure way, we want to generate the
     * following terms:
     *
     * a_2' = set_index(a_2, i2, y)
     * a_1' = set_index(a_1, i1, a_2')
     * a_0' = set_index(a_0, i0, a_1')
     *
     * Then we rename the top-level name ("a") to the final result:
     *
     * a = a_0'
     */

    Term* getter = getterRoot;

    while (true) {

        // Don't write a setter for a getter that already has a name; if the term has
        // a name then it's not part of this lexpr.
        if (getter->name != "")
            return desired;

        Term* result = write_setter_from_getter(branch, getter, desired);

        if (result == NULL)
            break;

        desired = result;
        getter = getter->input(0);
    }

    return desired;
}

void write_setter_chain_for_assign_term(Term* assignTerm)
{
    Branch* contents = nested_contents(assignTerm);
    clear_branch(contents);

    Term* result = write_setter_chain_from_getter_chain(contents,
        assignTerm->input(0), assignTerm->input(1));

    append_output_placeholder(contents, result);
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
