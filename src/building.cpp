// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "circa.h"
#include "heap_debugging.h"
#include "kernel.h"
#include "locals.h"
#include "names.h"
#include "parser.h"
#include "update_cascades.h"

namespace circa {

Term* apply(Branch* branch, Term* function, TermList const& inputs, std::string const& name)
{
    ca_assert(function != NULL);

    // If 'function' is actually a type, create a value instead.
    if (is_type(function)) {
        if (inputs.length() == 0) {
            Term* result = create_value(branch, as_type(function));
            result->setBoolProp("constructor", true);
            return result;
        } else if (inputs.length() == 1) {
            Term* result = apply(branch, CAST_FUNC, inputs);
            change_declared_type(result, as_type(function));
            return result;
        } else {
            internal_error("Constructors with multiple arguments not yet supported.");
        }
    }

    int outputCount = count_output_placeholders(branch);

    // Create the term
    Term* result = branch->appendNew();

    // Position the term before any output_placeholder terms.
    if (function != OUTPUT_PLACEHOLDER_FUNC && outputCount > 0)
        branch->move(result, branch->length() - outputCount - 1);

    if (name != "")
        branch->bindName(result, name);

    for (int i=0; i < inputs.length(); i++)
        set_input(result, i, inputs[i]);

    // change_function will also update the declared type.
    change_function(result, function);

    update_unique_name(result);
    on_inputs_changed(result);
    update_extra_outputs(result);

    return result;
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
        term->inputs[0] = Term::Input(inputs[i]);
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
    respecialize_type(term);

    // Don't append user for certain functions. Need to make this more robust.
    if (function != NULL
            && function != BUILTIN_FUNCS.value
            && function != INPUT_PLACEHOLDER_FUNC) {
        append_user(term, function);
    }

    // Possibly insert a state input for the enclosing subroutine.
    if (is_function_stateful(function))
        on_stateful_function_call_created(term);
}


void unsafe_change_type(Term *term, Type *type)
{
    ca_assert(type != NULL);

    term->type = type;
}

void change_declared_type(Term *term, Type *newType)
{
    ca_assert(term != NULL);
    ca_assert(newType != NULL);

    if (term->type == newType)
        return;

    term->type = newType;

    set_null((TaggedValue*) term);

    // TODO: Don't call create() here
    create(newType, term);

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

    if ((term->owningBranch != NULL) &&
            (term->owningBranch->get(term->name) == term)) {
        term->owningBranch->names.remove(term->name);
        term->name = "";
        term->owningBranch->bindName(term, name);
    }

    term->name = name;
}

Term* create_duplicate(Branch* branch, Term* original, std::string const& name, bool copyBranches)
{
    ca_assert(original != NULL);

    TermList inputs;
    original->inputsToList(inputs);

    Term* term = apply(branch, original->function, inputs, name);
    change_declared_type(term, original->type);
    //create(original->value_type, (TaggedValue*) term);

    copy(original, term);

    if (copyBranches)
        duplicate_branch(nested_contents(original), nested_contents(term));

    term->sourceLoc = original->sourceLoc;
    copy(&original->properties, &term->properties);

    // Special case for certain types, update declaringType
    if (is_value(term) && is_type(term))
        as_type(term)->declaringTerm = term;
    if (is_value(term) && is_function(term) && as_function(term) != NULL)
        as_function(term)->declaringTerm = term;

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

    Term *term = branch->appendNew();

    if (name != "")
        branch->bindName(term, name);

    change_function(term, BUILTIN_FUNCS.value);
    change_declared_type(term, type);
    create(type, (TaggedValue*) term);
    update_unique_name(term);

    return term;
}

Term* create_value(Branch* branch, std::string const& typeName, std::string const& name)
{
    Term* type = NULL;

    type = find_name(branch, typeName.c_str());

    if (type == NULL)
        internal_error("Couldn't find type: "+typeName);

    return create_value(branch, as_type(type), name);
}

Term* create_value(Branch* branch, TaggedValue* initialValue, std::string const& name)
{
    Term* term = create_value(branch, initialValue->value_type, name);
    copy(initialValue, term);
    return term;
}

Term* create_string(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, &STRING_T, name);
    set_string(term, s);
    return term;
}

Term* create_int(Branch* branch, int i, std::string const& name)
{
    Term* term = create_value(branch, &INT_T, name);
    set_int(term, i);
    return term;
}

Term* create_float(Branch* branch, float f, std::string const& name)
{
    Term* term = create_value(branch, &FLOAT_T, name);
    set_float(term, f);
    return term;
}

Term* create_bool(Branch* branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, &BOOL_T, name);
    set_bool(term, b);
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
    return apply(owner, BRANCH_FUNC, TermList(), name)->contents();
}

Branch* create_namespace(Branch* branch, std::string const& name)
{
    return apply(branch, NAMESPACE_FUNC, TermList(), name)->contents();
}
Branch* create_branch_unevaluated(Branch* owner, const char* name)
{
    return nested_contents(apply(owner, BRANCH_UNEVALUATED_FUNC, TermList(), name));
}

Term* create_type(Branch* branch, std::string name)
{
    Term* term = create_value(branch, &TYPE_T);

    if (name != "") {
        as_type(term)->name = name;
        branch->bindName(term, name);
    }

    return term;
}

Term* create_type_value(Branch* branch, Type* value, std::string const& name)
{
    Term* term = create_value(branch, &TYPE_T, name);
    set_type(term, value);
    return term;
}

Term* create_symbol_value(Branch* branch, int value, std::string const& name)
{
    Term* term = create_value(branch, &SYMBOL_T, name);
    set_symbol(term, value);
    return term;
}

Term* duplicate_value(Branch* branch, Term* term)
{
    Term* dup = create_value(branch, term->type);
    copy(term, dup);
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
    if (term == NULL || term->function != INPUT_PLACEHOLDER_FUNC)
        return NULL;
    return term;
}

Term* get_output_placeholder(Branch* branch, int index)
{
    if (index >= branch->length())
        return NULL;
    Term* term = branch->getFromEnd(index);
    if (term == NULL || term->function != OUTPUT_PLACEHOLDER_FUNC)
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
    return term->function == INPUT_PLACEHOLDER_FUNC;
}
bool is_output_placeholder(Term* term)
{
    return term->function == OUTPUT_PLACEHOLDER_FUNC;
}
bool has_variable_args(Branch* branch)
{
    Term* input0 = get_input_placeholder(branch, 0);
    if (input0 == NULL)
        return false;
    return input0->boolPropOptional("multiple", false);
}    
Term* find_input_placeholder_with_name(Branch* branch, const char* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            return NULL;
        if (placeholder->name == name)
            return placeholder;
    }
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
    Term* term = apply(branch, INPUT_PLACEHOLDER_FUNC, TermList());
    branch->move(term, count);
    return term;
}
Term* append_output_placeholder(Branch* branch, Term* result)
{
    int count = count_output_placeholders(branch);
    Term* term = apply(branch, OUTPUT_PLACEHOLDER_FUNC, TermList(result));
    branch->move(term, branch->length() - count - 1);
    return term;
}
Term* prepend_output_placeholder(Branch* branch, Term* result)
{
    return apply(branch, OUTPUT_PLACEHOLDER_FUNC, TermList(result));
}

Branch* term_get_function_details(Term* call)
{
    if (call->function == IF_BLOCK_FUNC || call->function == FOR_FUNC)
        return nested_contents(call);

    return function_get_contents(as_function(call->function));
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
        Term* existing = branch->getSafe(term->index + index);
        if (existing != NULL && existing->function == EXTRA_OUTPUT_FUNC)
            extra_output = existing;
        
        if (extra_output == NULL)
            extra_output = apply(term->owningBranch, EXTRA_OUTPUT_FUNC, TermList(term), name);
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
    if (term->function != INPUT_PLACEHOLDER_FUNC)
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
    if (term->function != OUTPUT_PLACEHOLDER_FUNC)
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
        if (term->function == INPUT_PLACEHOLDER_FUNC && function_is_state_input(term))
            return term;
        if (term->function == BUILTIN_FUNCS.pack_state
                || term->function == BUILTIN_FUNCS.pack_state_list_n)
            return term;
    }
    return NULL;
}

Term* find_open_state_result(Term* location)
{
    return find_open_state_result(location->owningBranch, location->index);
}

Term* find_or_create_open_state_result(Branch* branch, int position)
{
    Term* term = find_open_state_result(branch, position);
    if (term == NULL)
        return append_state_input(branch);
    else
        return term;
}

void check_to_insert_implicit_inputs(Term* term)
{
    if (!is_function(term->function))
        return;

    Term* stateInput = find_state_input(term_get_function_details(term));

    if (stateInput != NULL && !term_is_state_input(term, stateInput->index)) {

        Term* container = find_or_create_open_state_result(term->owningBranch, term->index);

        Term* unpack = apply(term->owningBranch, BUILTIN_FUNCS.unpack_state, TermList(container));
        unpack->setStringProp("field", unique_name(term));
        hide_from_source(unpack);
        term->owningBranch->move(unpack, term->index);

        insert_input(term, stateInput->index, unpack);
        set_bool(term->inputInfo(0)->properties.insert("state"), true);
        set_bool(term->inputInfo(0)->properties.insert("hidden"), true);
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
        apply(rebinds, COND_FUNC, TermList(rebindCondition, pos, neg), name);
    }
}

void post_compile_term(Term* term)
{
    if (term->function == NULL || !is_function(term->function))
        return;

    Function* attrs = as_function(term->function);
    if (attrs == NULL)
        return;

    Function::PostCompile func = attrs->postCompile;

    if (func != NULL)
        func(term);

    update_extra_outputs(term);

    Term* stateOutput = find_extra_output_for_state(term);

    // Possibly append a pack_state() call for a state extra output.
    Term* unpack = find_input_with_function(term, BUILTIN_FUNCS.unpack_state);
    if (stateOutput != NULL && unpack != NULL) {
        Term* container = unpack->input(0);
        Term* pack = apply(term->owningBranch, BUILTIN_FUNCS.pack_state, TermList(container, stateOutput));
        pack->setStringProp("field", unpack->stringProp("field"));
        hide_from_source(pack);
    }

    // If this term rebinds the name of a declared state var, then it also needs
    // a pack_state() call.
    Branch* branch = term->owningBranch;
    if (term->name != "") {
        if (branch->findFirstBinding(term->name)->function == DECLARED_STATE_FUNC) {
            Term* pack = apply(branch, BUILTIN_FUNCS.pack_state, TermList(
                find_open_state_result(branch, branch->length()),
                term));
            pack->setStringProp("field", term->name);
            move_after(pack, term);
        }
    }
}

void finish_minor_branch(Branch* branch)
{
    // Create an output_placeholder for state, if necessary
    Term* openState = find_open_state_result(branch, branch->length());
    if (openState != NULL)
        insert_state_output(branch);
}

Term* find_last_non_comment_expression(Branch* branch)
{
    for (int i = branch->length() - 1; i >= 0; i--) {
        if (branch->get(i) == NULL)
            continue;
        if (branch->get(i)->function == OUTPUT_PLACEHOLDER_FUNC)
            continue;
        if (branch->get(i)->function == INPUT_PLACEHOLDER_FUNC)
            continue;
        if (branch->get(i)->name == "#outer_rebinds")
            continue;
        if (branch->get(i)->function != COMMENT_FUNC)
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

bool branch_creates_stack_frame(Branch* branch)
{
    if (branch->owningTerm == NULL)
        return true;

    // Special case that should be removed; #outer_rebinds doesn't create a stack
    // frame.
    if (branch->owningTerm->name == "#outer_rebinds")
        return false;

    return as_function(branch->owningTerm->function)->createsStackFrame;
}

int get_frame_distance(Branch* frame, Term* input)
{
    if (input == NULL)
        return -1;

    Branch* inputFrame = input->owningBranch;

    // If the input's branch doesn't create a separate stack frame, then look
    // at the parent branch.
    if (!branch_creates_stack_frame(inputFrame))
        inputFrame = get_parent_branch(inputFrame);

    // Walk upward from 'term' until we find the common branch.
    int distance = 0;
    while (frame != inputFrame) {

        if (branch_creates_stack_frame(frame))
            distance++;

        frame = get_parent_branch(frame);

        if (frame == NULL)
            return -1;
    }
    return distance;
}

int get_frame_distance(Term* term, Term* input)
{
    return get_frame_distance(term->owningBranch, input);
}

void write_stack_input_instruction(Branch* callingFrame, Term* input, TaggedValue* isn)
{
    change_type(isn, &StackVariableIsn_t);
    int relativeFrame = get_frame_distance(callingFrame, input);

    // Special case: if a term in a #joining branch is trying to reach a neighboring
    // branch, then that's okay.
    if (relativeFrame == -1 && callingFrame->owningTerm->name == "#joining")
        relativeFrame = 0;
    
    isn->value_data.asint = 0;
    isn->value_data.asint += relativeFrame << 16;
    isn->value_data.asint += (input->index % 0xffff);
}

void write_input_instruction(Term* caller, Term* input, TaggedValue* isn)
{
    if (input == NULL) {
        set_pointer(isn, &NullInputIsn_t, NULL);
    } else if (is_value(input)) {
        set_pointer(isn, &GlobalVariableIsn_t, input);
    } else {
        write_stack_input_instruction(caller->owningBranch, input, isn);
    }
}

bool term_is_state_input(Term* term, int index)
{
    if (index >= term->numInputs())
        return false;
    TaggedValue* prop = term->inputInfo(index)->properties.get("state");
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
        if (placeholder->function != OUTPUT_PLACEHOLDER_FUNC)
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

    term = apply(branch, INPUT_PLACEHOLDER_FUNC, TermList());
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
    if (term != NULL)
        return term;
    term = apply(branch, OUTPUT_PLACEHOLDER_FUNC,
        TermList(find_open_state_result(branch, branch->length())));
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

ListData* write_input_instruction_list(Term* caller, ListData* list)
{
    list = list_resize(list, 0);

    int declaredIndex = 0;

    for (int i=0; i < caller->numInputs(); i++) {

        Term* placeholder = term_get_input_placeholder(caller, declaredIndex);
        ca_assert(placeholder != NULL);

        write_input_instruction(caller, caller->input(i), list_append(&list));

        // Advance to the next declaredIndex, unless we found a :multiple input.
        if (!function_is_multiple_input(placeholder))
            declaredIndex++;
    }

    return list;
}

ListData* write_output_instruction_list(Term* caller, ListData* list)
{
    list = list_resize(list, 1);

    // Always write a primary output that corresponds to the caller's register.
    write_input_instruction(caller, caller, list_get_index(list, 0));

    // Write instructions for nearby extra_output() calls.
    for (int i=1; ; i++) {
        Term* receiver = caller->owningBranch->getSafe(caller->index + i);
        if (receiver == NULL || receiver->function != EXTRA_OUTPUT_FUNC)
            break;

        write_input_instruction(caller, receiver, list_append(&list));
    }

    return list;
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
    int pos = position->index + 1;
    Branch* branch = movee->owningBranch;
    while (branch->get(pos) != NULL && branch->get(pos)->function == EXTRA_OUTPUT_FUNC)
        pos++;
    branch->move(movee, pos);
}

void move_after_inputs(Term* term)
{
    Branch* branch = term->owningBranch;
    int inputCount = count_input_placeholders(branch);
    branch->move(term, inputCount);
}
void move_before_outputs(Term* term)
{
    Branch* branch = term->owningBranch;
    int outputCount = count_output_placeholders(branch);
    branch->move(term, branch->length() - outputCount - 1);
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

    Term* output = apply(branch, OUTPUT_PLACEHOLDER_FUNC, TermList(result));
    output->setBoolProp("state", true);
}

Term* find_intermediate_result_for_output(Term* location, Term* output)
{
    if (is_state_input(output)) {
        return find_open_state_result(location);
    } else if (output->name != "") {
        return find_name_at(location, output->name.c_str());
    } else {
        return NULL;
    }
}

void update_exit_points(Branch* branch)
{
    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = it.current();

        if (term->function == RETURN_FUNC) {
            for (int i=1;; i++) {
                Term* output = get_output_placeholder(branch, i);
                if (output == NULL)
                    break;
                set_input(term, i, find_intermediate_result_for_output(term, output));
            }
        }
    }
}

} // namespace circa
