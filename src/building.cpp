// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"
#include "circa.h"
#include "debug_valid_objects.h"
#include "locals.h"
#include "names.h"
#include "parser.h"

namespace circa {

Term* apply(Branch& branch, Term* function, RefList const& inputs, std::string const& name)
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
            change_type(result, function);
            return result;
        } else {
            internal_error("Constructors with multiple arguments not yet supported.");
        }
    }

    // Create the term
    Term* result = branch.appendNew();

    result->function = function;

    if (name != "")
        branch.bindName(result, name);

    RefList _inputs = inputs;

    // If the function takes a state input, and there aren't enough inputs, then prepend
    // a NULL input for state.
    if (is_function_stateful(function)
            && _inputs.length() == function_t::num_inputs(function) - 1) {
        _inputs.prepend(NULL);
    }

    for (int i=0; i < _inputs.length(); i++)
        set_input(result, i, _inputs[i]);

    Term* outputType = function_get_specialized_output_type(function, result);

    ca_assert(outputType != NULL);
    ca_assert(is_type(outputType));

    change_type(result, outputType);

    post_input_change(result);

    update_unique_name(result);

    update_locals_index_for_new_term(result);

    return result;
}

void set_input2(Term* term, int index, Term* input, int outputIndex)
{
    assert_valid_term(term);
    assert_valid_term(input);

    Ref previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    while (index >= term->numInputs())
        term->inputs.push_back(NULL);

    term->inputs[index] = Term::Input(input, outputIndex);

    // Add 'term' to the user list of 'input'
    if (input != NULL && term != input)
        input->users.appendUnique(term);

    // Check if we should remove 'term' from the user list of previousInput
    if (previousInput != NULL && !is_actually_using(previousInput, term))
        previousInput->users.remove(term);

    // Don't do post_input_change here, caller must call it.
}

void set_input(Term* term, int index, Term* input)
{
    assert_valid_term(term);
    assert_valid_term(input);

    Ref previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    while (index >= term->numInputs())
        term->inputs.push_back(NULL);

    term->inputs[index] = Term::Input(input);

    // Add 'term' to the user list of 'input'
    if (input != NULL && term != input)
        input->users.appendUnique(term);

    // Check if we should remove 'term' from the user list of previousInput
    if (previousInput != NULL && !is_actually_using(previousInput, term))
        previousInput->users.remove(term);

    post_input_change(term);
}

void set_inputs(Term* term, RefList const& inputs)
{
    assert_valid_term(term);

    Term::InputList previousInputs = term->inputs;

    term->inputs.resize(inputs.length());
    for (int i=0; i < inputs.length(); i++)
        term->inputs[0] = Term::Input(inputs[i]);

    // Add 'term' as a user to these new inputs
    for (int i=0; i < inputs.length(); i++)
        if (inputs[i] != NULL)
            inputs[i]->users.appendUnique(term);

    // Check to remove 'term' from user list of any previous inputs
    for (size_t i=0; i < previousInputs.size(); i++) {
        Term* previousInput = previousInputs[i].term;
        if (previousInput != NULL && !is_actually_using(previousInput, term))
            previousInput->users.remove(term);
    }

    post_input_change(term);
}

void insert_input(Term* term, Term* input)
{
    term->inputs.insert(term->inputs.begin(), Term::Input(NULL));
    set_input(term, 0, input);
}

void post_input_change(Term* term)
{
    FunctionAttrs* attrs = get_function_attrs(term->function);
    if (attrs == NULL)
        return;

    FunctionAttrs::PostInputChange func = attrs->postInputChange;

    if (func)
        func(term);
}

bool is_actually_using(Term* user, Term* usee)
{
    for (int i=0; i < usee->numInputs(); i++)
        if (usee->input(i) == user)
            return true;

    return false;
}

void remove_from_users(Term* term)
{
    for (int i=0; i < term->numInputs(); i++) {
        Term* user = term->input(i);
        if (user == NULL) continue;
        user->users.remove(term);
    }
}

Term* create_duplicate(Branch& branch, Term* original, std::string const& name, bool copyBranches)
{
    ca_assert(original != NULL);

    RefList inputs;
    original->inputsToList(&inputs);

    Term* term = apply(branch, original->function, inputs, name);
    change_type(term, original->type);
    change_type((TaggedValue*) term, original->value_type);

    copy(original, term);

    if (copyBranches)
        duplicate_branch(original->nestedContents, term->nestedContents);

    term->sourceLoc = original->sourceLoc;
    copy(&original->properties, &term->properties);

    return term;
}

Term* apply(Branch& branch, std::string const& functionName, RefList const& inputs, std::string const& name)
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

    term->function = VALUE_FUNC;
    term->type = type;
    change_type(term, type);
    update_unique_name(term);
    update_locals_index_for_new_term(term);

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
    Term* result = apply(branch, get_global("get_state_field"),
            RefList(NULL, defaultValue), name);
    change_type(result, type);
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
    return apply(owner, BRANCH_FUNC, RefList(), name)->nestedContents;
}

Branch& create_namespace(Branch& branch, std::string const& name)
{
    return apply(branch, NAMESPACE_FUNC, RefList(), name)->nestedContents;
}

Term* create_type(Branch& branch, std::string name)
{
    Term* term = create_value(branch, TYPE_TYPE);

    if (name != "") {
        as_type(term).name = name;
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
        change_type(existing, type);
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
    rebinds.clear();

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(source, reboundNames);

    Branch& outerScope = *source.owningTerm->owningBranch;
    for (unsigned i=0; i < reboundNames.size(); i++) {
        std::string name = reboundNames[i];
        Term* outerVersion = find_named(outerScope, name);
        Term* innerVersion = source[name];

        Term* pos = outsidePositive ? outerVersion : innerVersion;
        Term* neg = outsidePositive ? innerVersion : outerVersion ;
        apply(rebinds, COND_FUNC, RefList(rebindCondition, pos, neg), name);
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
            Term* outputCopy = apply(owningBranch, COPY_FUNC, RefList(), name);
            set_input2(outputCopy, 0, term, outputIndex);

            possibly_respecialize_type(outputCopy);
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

                Term* output = apply(owningBranch, COPY_FUNC, RefList(), name);
                set_input2(output, 0, term, outputIndex);

                possibly_respecialize_type(output);
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

    post_compile_term(apply(branch, FINISH_MINOR_BRANCH_FUNC, RefList()));
}

void check_to_add_branch_finish_term(Branch& branch, int previousLastTerm)
{
    for (int i=previousLastTerm; i < branch.length(); i++) {

        if (branch[i] == NULL)
            continue;

        if (branch[i]->function == GET_STATE_FIELD_FUNC) {
            Term* term = apply(branch, FINISH_MINOR_BRANCH_FUNC, RefList());
            update_branch_finish_term(term);
            break;
        }
    }
}

void update_branch_finish_term(Term* term)
{
    post_compile_term(term);
}

} // namespace circa
