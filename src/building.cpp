// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "circa.h"
#include "debug_valid_objects.h"
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

    return result;
}

void set_input(Term* term, int index, Term* input)
{
    assert_valid_term(term);

    Ref previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    term->inputs.setAt(index, input);

    if (index >= term->inputInfoList.length())
        term->inputInfoList.resize(index+1);

    // Add 'term' to the user list of input
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

    RefList previousInputs = term->inputs;

    term->inputs = inputs;

    term->inputInfoList.resize(inputs.length());

    // Add 'term' as a user to these new inputs
    for (int i=0; i < inputs.length(); i++)
        if (inputs[i] != NULL)
            inputs[i]->users.appendUnique(term);

    // Check to remove 'term' from user list of any previous inputs
    for (int i=0; i < previousInputs.length(); i++) {
        Term* previousInput = previousInputs[i];
        if (previousInput != NULL && !is_actually_using(previousInput, term))
            previousInput->users.remove(term);
    }

    post_input_change(term);
}

int get_input_relative_scope(Term* term, int index)
{
    Term* inputTerm = term->input(index);

    if (inputTerm == NULL)
        return -1;

    Branch* rootScope = inputTerm->owningBranch;

    ca_assert(rootScope != NULL);

    // Special case for if-blocks: if a join term is trying to reach a term inside
    // an if-branch, then it's relative scope 0.
    
    Term* termParent = get_parent_term(term);
    Term* inputParent = get_parent_term(inputTerm);
    Term* input2ndParent = inputParent == NULL ? NULL : get_parent_term(inputParent);

    if (termParent != NULL && termParent->name == "#joining"
            && input2ndParent != NULL && input2ndParent->function == IF_BLOCK_FUNC) {
        return 0;
    }

    // Otherwise, if a term is inside an if_block #joining, use the if_block's parent
    // as rootScope.
    if (inputParent != NULL && inputParent->name == "#joining"
            && input2ndParent != NULL && input2ndParent->function == IF_BLOCK_FUNC)
        rootScope = input2ndParent->owningBranch;

    // Ditto for a for_loop #outer_rebind
    if (inputParent != NULL && inputParent->name == "#outer_rebinds"
            && input2ndParent != NULL && input2ndParent->function == FOR_FUNC)
        rootScope = input2ndParent->owningBranch;

    // For a for_loop #inner_rebind, use the for-loop contents as the root scope.
    if (inputParent != NULL && inputParent->name == "#inner_rebinds"
            && input2ndParent != NULL && input2ndParent->function == FOR_FUNC)
        rootScope = inputParent->owningBranch;

    // Walk upwards from 'term' until we find the root branch.
    int relativeScope = 0;
    Branch* scope = term->owningBranch;
    while (scope != rootScope) {

        if (scope == NULL && rootScope != NULL) {
            //FIXME internal_error("Couldn't reach root scope from term");
            return -1;
        }

        // In certain cases, we don't count a scope layer.
        bool countLayer = true;
        Term* parentTerm = scope->owningTerm;

        if (parentTerm == NULL)
            countLayer = true;
        else if (parentTerm->function == IF_BLOCK_FUNC)
            countLayer = false;
        else if (parentTerm->name == "#inner_rebinds")
            countLayer = false;
        else if (parentTerm->name == "#outer_rebinds")
            countLayer = false;
        else if (parentTerm->function == NAMESPACE_FUNC)
            countLayer = false;

        if (countLayer)
            relativeScope++;

        scope = get_parent_branch(*scope);
    }

    ca_assert(relativeScope >= 0);

    return relativeScope;
}

void post_input_change(Term* term)
{
    for (int i=0; i < term->numInputs(); i++)
        term->inputInfo(i).relativeScope = get_input_relative_scope(term, i);

    if (is_function(term->function)) {
        FunctionAttrs::PostInputChange func =
            function_t::get_attrs(term->function).postInputChange;
        if (func)
            func(term);
    }
}

bool is_actually_using(Term* user, Term* usee)
{
    for (int i=0; i < usee->numInputs(); i++)
        if (usee->input(i) == user)
            return true;

    return false;
}

void clear_all_users(Term* term)
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

    Term* term = apply(branch, original->function, original->inputs, name);
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
    reset(term);
    update_unique_name(term);

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
    Term* nameTerm = create_string(branch, name);
    hide_from_source(nameTerm);

    Term* result = apply(branch, get_global("get_state_field"),
            RefList(NULL, nameTerm, defaultValue), name);
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
    // Check for overloaded_function
    if (term->function == OVERLOADED_FUNCTION_FUNC) {
        overloaded_function::post_compile_setup_overloaded_function(term);
    }

    if (term->function == INCLUDE_FUNC) {
        include_function::preload_script(term);
    }
}

} // namespace circa
