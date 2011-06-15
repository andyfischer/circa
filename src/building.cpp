// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"
#include "building.h"
#include "circa.h"
#include "heap_debugging.h"
#include "locals.h"
#include "names.h"
#include "parser.h"
#include "update_cascades.h"

namespace circa {

Term* apply(Branch& branch, Term* function, TermList const& inputs, std::string const& name)
{
    ca_assert(function != NULL);

    // If 'function' is actually a type, create a value instead.
    if (is_type(function)) {
        if (inputs.length() == 0) {
            Term* result = create_value(branch, function);
            result->setBoolProp("constructor", true);
            return result;
        } else if (inputs.length() == 1) {
            Term* result = apply(branch, CAST_FUNC, inputs);
            change_declared_type(result, function);
            return result;
        } else {
            internal_error("Constructors with multiple arguments not yet supported.");
        }
    }

    // Create the term
    Term* result = branch.appendNew();

    if (name != "")
        branch.bindName(result, name);

    TermList _inputs = inputs;

    // If the function takes a state input, and there aren't enough inputs, then prepend
    // a NULL input for state.
    if (is_function_stateful(function)
            && _inputs.length() == function_t::num_inputs(function) - 1) {
        _inputs.prepend(NULL);
    }

    for (int i=0; i < _inputs.length(); i++)
        set_input(result, i, _inputs[i]);

    // change_function will also update the declared type.
    change_function(result, function);

    update_unique_name(result);
    on_inputs_changed(result);
    update_locals_index_for_new_term(result);
    update_input_instructions(result);

    if (is_get_state(result) || has_implicit_state(result))
        mark_branch_as_having_inlined_state(branch);

    return result;
}

void set_input2(Term* term, int index, Term* input, int outputIndex)
{
    assert_valid_term(term);
    assert_valid_term(input);

    Term* previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    while (index >= term->numInputs())
        term->inputs.push_back(NULL);

    term->inputs[index] = Term::Input(input, outputIndex);

    // Add 'term' to the user list of 'input'
    append_user(term, input);

    // Check if we should remove 'term' from the user list of previousInput
    possibly_prune_user_list(term, previousInput);

    mark_inputs_changed(term);
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

Term* create_duplicate(Branch& branch, Term* original, std::string const& name, bool copyBranches)
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

    return term;
}

Term* apply(Branch& branch, std::string const& functionName, TermList const& inputs, std::string const& name)
{
    Term* function = find_named(branch, functionName);
    if (function == NULL)
        internal_error("function not found: "+functionName);

    Term* result = apply(branch, function, inputs, name);
    result->setStringProp("syntax:functionName", functionName);
    return result;
}

Term* create_value(Branch& branch, Term* type, std::string const& name)
{
    // This function is safe to call while bootstrapping.
    ca_assert(type != NULL);
    ca_assert(is_type(type));

    Term *term = branch.appendNew();

    if (name != "")
        branch.bindName(term, name);

    change_function(term, VALUE_FUNC);
    change_declared_type(term, type);
    change_type((TaggedValue*) term, unbox_type(type));
    update_unique_name(term);
    update_locals_index_for_new_term(term);
    update_input_instructions(term);

    return term;
}

Term* create_value(Branch& branch, std::string const& typeName, std::string const& name)
{
    Term* type = NULL;

    type = find_named(branch, typeName);

    if (type == NULL)
        internal_error("Couldn't find type: "+typeName);

    return create_value(branch, type, name);
}

Term* create_stateful_value(Branch& branch, Term* type, Term* defaultValue,
        std::string const& name)
{
    Term* result = apply(branch, GET_STATE_FIELD_FUNC,
            TermList(NULL, defaultValue), name);
    change_declared_type(result, type);
    return result;
}

Term* create_string(Branch& branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, STRING_TYPE, name);
    set_string(term, s);
    return term;
}

Term* create_int(Branch& branch, int i, std::string const& name)
{
    Term* term = create_value(branch, INT_TYPE, name);
    set_int(term, i);
    return term;
}

Term* create_float(Branch& branch, float f, std::string const& name)
{
    Term* term = create_value(branch, FLOAT_TYPE, name);
    set_float(term, f);
    return term;
}

Term* create_bool(Branch& branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, BOOL_TYPE, name);
    set_bool(term, b);
    return term;
}

Term* create_ref(Branch& branch, Term* ref, std::string const& name)
{
    Term* term = create_value(branch, REF_TYPE, name);
    set_ref(term, ref);
    return term;
}
Term* create_void(Branch& branch, std::string const& name)
{
    return create_value(branch, VOID_TYPE, name);
}

Term* create_list(Branch& branch, std::string const& name)
{
    Term* term = create_value(branch, LIST_TYPE, name);
    return term;
}

Branch& create_branch(Branch& owner, std::string const& name)
{
    return apply(owner, BRANCH_FUNC, TermList(), name)->contents();
}

Branch& create_namespace(Branch& branch, std::string const& name)
{
    return apply(branch, NAMESPACE_FUNC, TermList(), name)->contents();
}

Term* create_type(Branch& branch, std::string name)
{
    Term* term = create_value(branch, TYPE_TYPE);

    if (name != "") {
        as_type(term)->name = name;
        branch.bindName(term, name);
    }

    return term;
}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* type = create_type(branch, name);
    return type;
}

Term* duplicate_value(Branch& branch, Term* term)
{
    Term* dup = create_value(branch, term->type);
    copy(term, dup);
    return dup;
}

Term* procure_value(Branch& branch, Term* type, std::string const& name)
{
    Term* existing = branch[name];
    if (existing == NULL)
        existing = create_value(branch, type, name);
    else
        change_declared_type(existing, type);
    return existing;
}

Term* procure_int(Branch& branch, std::string const& name)
{
    return procure_value(branch, INT_TYPE, name);
}

Term* procure_float(Branch& branch, std::string const& name)
{
    return procure_value(branch, FLOAT_TYPE, name);
}

Term* procure_bool(Branch& branch, std::string const& name)
{
    return procure_value(branch, BOOL_TYPE, name);
}

void set_step(Term* term, float step)
{
    term->setFloatProp("step", step);
}

float get_step(Term* term)
{
    return term->floatPropOptional("step", 1.0);
}

void create_rebind_branch(Branch& rebinds, Branch& source, Term* rebindCondition, bool outsidePositive)
{
    clear_branch(&rebinds);

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(source, reboundNames);

    Branch& outerScope = *source.owningTerm->owningBranch;
    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Term* outerVersion = find_named(outerScope, name);
        Term* innerVersion = source[name];

        Term* pos = outsidePositive ? outerVersion : innerVersion;
        Term* neg = outsidePositive ? innerVersion : outerVersion ;
        apply(rebinds, COND_FUNC, TermList(rebindCondition, pos, neg), name);
    }
}

void post_compile_term(Term* term)
{
    if (term->function == NULL)
        return;

    FunctionAttrs* attrs = get_function_attrs(term->function);
    if (attrs == NULL)
        return;

    FunctionAttrs::PostCompile func = attrs->postCompile;

    if (func != NULL) {
        func(term);
        return;
    }

    // Default behavior for postCompile..
    
    // If the function has multiple outputs, and any of these outputs have names,
    // then copy these outputs to named terms. This is a workaround until we can
    // fully support terms with multiple output names.
    Branch& owningBranch = *term->owningBranch;
    int numOutputs = get_output_count(term);
    for (int outputIndex=1; outputIndex < numOutputs; outputIndex++) {
        const char* name = get_output_name(term, outputIndex);
        if (strcmp(name, "") != 0) {
            Term* outputCopy = apply(owningBranch, COPY_FUNC, TermList(), name);
            set_input2(outputCopy, 0, term, outputIndex);

            respecialize_type(outputCopy);
            owningBranch.bindName(outputCopy, name);
        }
    }

    #if 0
    int outputIndex = 1;
    for (int i=0; i < term->numInputs(); i++) {
        if (function_can_rebind_input(term->function, i)) {
            if (function_call_rebinds_input(term, i)
                    && term->input(i) != NULL
                    && term->input(i)->name != "") {

                std::string name = term->input(i)->name;

                ca_assert(name == get_output_name(term, outputIndex));

                Term* output = apply(owningBranch, COPY_FUNC, TermList(), name);
                set_input2(output, 0, term, outputIndex);

                respecialize_type(output);
                owningBranch.bindName(output, term->input(i)->name);

            }
            outputIndex++;
        }
    }
    #endif
}

void finish_minor_branch(Branch& branch)
{
    if (branch.length() > 0
            && branch[branch.length()-1]->function == FINISH_MINOR_BRANCH_FUNC)
        return;

    // Check if there are any state vars in this branch
    bool anyStateVars = false;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term->function == GET_STATE_FIELD_FUNC) {
            anyStateVars = true;
            break;
        }
    }

    if (!anyStateVars)
        return;

    post_compile_term(apply(branch, FINISH_MINOR_BRANCH_FUNC, TermList()));
}

void check_to_add_branch_finish_term(Branch& branch, int previousLastTerm)
{
    for (int i=previousLastTerm; i < branch.length(); i++) {

        if (branch[i] == NULL)
            continue;

        if (branch[i]->function == GET_STATE_FIELD_FUNC) {
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

Term* find_last_non_comment_expression(Branch& branch)
{
    for (int i = branch.length() - 2; i >= 0; i--)
        if (branch[i] != NULL && branch[i]->function != COMMENT_FUNC)
            return branch[i];
    return NULL;
}

} // namespace circa
