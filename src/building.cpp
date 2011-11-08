// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "building.h"
#include "circa.h"
#include "heap_debugging.h"
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

    // Create the term
    Term* result = branch->appendNew();

    if (name != "")
        branch->bindName(result, name);

    for (int i=0; i < inputs.length(); i++)
        set_input(result, i, inputs[i]);

    // change_function will also update the declared type.
    change_function(result, function);

    update_unique_name(result);
    on_inputs_changed(result);

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

    term->inputs[index] = Term::Input(input);

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

bool is_actually_using(Term* user, Term* usee)
{
    for (int i=0; i < user->numDependencies(); i++)
        if (user->dependency(i) == usee)
            return true;

    return false;
}

void append_user(Term* user, Term* usee)
{
    if (usee != NULL && user != NULL) {
        #if 0
        std::cout << "adding " << global_id(user) << " as user of "
            << global_id(usee)
            << std::endl;
        #endif
        usee->users.appendUnique(user);
    }
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
            && function != VALUE_FUNC
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

    // TODO: Don't call change_type here
    change_type(term, newType);

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
    change_type((TaggedValue*) term, original->value_type);

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

    change_function(term, VALUE_FUNC);
    change_declared_type(term, type);
    change_type((TaggedValue*) term, type);
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

Term* create_stateful_value(Branch* branch, Type* type, Term* defaultValue,
        std::string const& name)
{
    Term* result = apply(branch, GET_STATE_FIELD_FUNC,
            TermList(NULL, defaultValue), name);
    change_declared_type(result, type);
    return result;
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

Term* create_symbol_value(Branch* branch, TaggedValue* value, std::string const& name)
{
    Term* term = create_value(branch, &SYMBOL_T, name);
    copy(value, term);
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

    if (func != NULL) {
        func(term);
        return;
    }

    // Default behavior for postCompile..
    
    // If the function has multiple outputs, then create extra_output terms for all of
    // those outputs.
    Branch* owningBranch = term->owningBranch;
    int numOutputs = get_output_count(term);
    for (int outputIndex=1; outputIndex < numOutputs; outputIndex++) {
        const char* name = get_output_name(term, outputIndex);
        if (strcmp(name, "") == 0)
            continue;
        Term* outputCopy = apply(owningBranch, EXTRA_OUTPUT_FUNC, TermList(term), name);

        respecialize_type(outputCopy);
        owningBranch->bindName(outputCopy, name);
    }
}

void finish_minor_branch(Branch* branch)
{
    if (branch->length() > 0
            && branch->get(branch->length()-1)->function == FINISH_MINOR_BRANCH_FUNC)
        return;

    // Check if there are any state vars in this branch
    bool anyStateVars = false;

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term->function == GET_STATE_FIELD_FUNC) {
            anyStateVars = true;
            break;
        }
    }

    if (!anyStateVars)
        return;

    post_compile_term(apply(branch, FINISH_MINOR_BRANCH_FUNC, TermList()));
}

void check_to_add_branch_finish_term(Branch* branch, int previousLastTerm)
{
    for (int i=previousLastTerm; i < branch->length(); i++) {

        if (branch->get(i) == NULL)
            continue;

        if (branch->get(i)->function == GET_STATE_FIELD_FUNC) {
            Term* term = apply(branch, FINISH_MINOR_BRANCH_FUNC, TermList());
            update_branch_finish_term(term);
            break;
        }
    }
}

void update_branch_finish_term(Term* term)
{
    post_compile_term(term);
}

Term* find_last_non_comment_expression(Branch* branch)
{
    for (int i = branch->length() - 1; i >= 0; i--) {
        if (branch->get(i) == NULL)
            continue;
        if (branch->get(i)->name == "#outer_rebinds")
            continue;
        if (branch->get(i)->function != COMMENT_FUNC)
            return branch->get(i);
    }
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
    change_type_no_initialize(isn, &StackVariableIsn_t);
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
        set_null(isn);
    } else if (is_value(input)) {
        set_pointer(isn, &GlobalVariableIsn_t, input);
    } else {
        write_stack_input_instruction(caller->owningBranch, input, isn);
    }
}

void write_implicit_state_input_instruction(Term* caller, TaggedValue* isn)
{
    set_null(isn);
    change_type_no_initialize(isn, &ImplicitStateInputIsn_t);

    // Save the register index of the state input. Currently the state value
    // is read & written to the same register.

}

bool term_is_state_input(Term* term, int index)
{
    TaggedValue* prop = term->inputInfo(index)->properties.get("state");
    if (prop == NULL)
        return false;
    return as_bool(prop);
}

void append_input_instruction_error(List* errors, TaggedValue* type)
{
    if (errors == NULL)
        return;

    copy(type, errors->append());
}

ListData* write_input_instruction_list(Term* caller, ListData* list, List* errors)
{
    Function* func = as_function(caller->function);
    list = list_resize(list, 0);

    // Walk through each of the function's declared inputs, and write appropriate
    // instructions.
    int callerIndex = 0;

    for (int declaredIndex=0; ; declaredIndex++) {
        Term* inputPlaceholder = function_get_input_placeholder(func, declaredIndex);
        if (inputPlaceholder == NULL)
            break;

        if (function_is_state_input(inputPlaceholder)) {

            if (term_is_state_input(caller, callerIndex)) {
                // Explicit state input
                write_input_instruction(caller, caller->input(callerIndex), list_append(&list));
                callerIndex++;
            } else {
                // Implicit state input
                //write_implicit_state_instruction(caller);
                //callerIndex++;
            }

        } else if (function_is_multiple_input(inputPlaceholder)) {

            // Write the remainder of caller's arguments.
            while (callerIndex < caller->numInputs()) {

                write_input_instruction(caller, caller->input(callerIndex), list_append(&list));
                callerIndex++;
            }
            break;

        } else {
            // Write a normal input.
            if (callerIndex >= caller->numInputs()) {
                append_input_instruction_error(errors, &NotEnoughInputsSymbol);
                break;
            }

            write_input_instruction(caller, caller->input(callerIndex), list_append(&list));
            callerIndex++;
        }
    }

    // Check if we didn't look at all of the caller's inputs.
    if (callerIndex+1 < caller->numInputs())
        append_input_instruction_error(errors, &TooManyInputsSymbol);

    return list;
}

ListData* write_output_instruction_list(Term* caller, ListData* list, List* errors)
{
    Function* func = as_function(caller->function);
    list = list_resize(list, 1);

    // Always write a primary output that corresponds to the caller's register.
    write_input_instruction(caller, caller, list_get_index(list, 0));

    // Walk through the function's declared outputs to find "extra" outputs, write
    // appropriate instructions for those.
    for (int declaredIndex=1; ; declaredIndex++) {
        Term* outputPlaceholder = function_get_output_placeholder(func, declaredIndex);
        if (outputPlaceholder == NULL)
            break;

        int receiverIndex = caller->index + declaredIndex;
        Term* receiver = NULL;
        if (receiverIndex < caller->owningBranch->length())
            receiver = caller->owningBranch->get(receiverIndex);

        if (receiver == NULL || receiver->function != EXTRA_OUTPUT_FUNC) {
            append_input_instruction_error(errors, &ExtraOutputNotFoundSymbol);
            break;
        }

        write_input_instruction(caller, receiver, list_append(&list));
    }

    return list;
}

} // namespace circa
