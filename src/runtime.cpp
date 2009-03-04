// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "debug.h"
#include "function.h"
#include "introspection.h"
#include "runtime.h"
#include "syntax.h"
#include "type.h"
#include "values.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, RefList const& inputs)
{
    assert_good_pointer(function);

    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    term->function = function;
    term->needsUpdate = true;

    Function& functionData = as_function(function);

    Term* outputType = functionData.outputType;

    if (outputType == NULL)
        throw std::runtime_error("outputType is NULL");
        
    if (!is_type(outputType))
        throw std::runtime_error(outputType->name + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    Term* stateType = functionData.stateType;
    if (stateType != NULL) {
        if (!is_type(stateType))
            throw std::runtime_error(outputType->name + " is not a type");
        term->state = create_value(NULL, stateType);
    }

    for (unsigned int i=0; i < inputs.count(); i++)
        set_input(term, i, inputs[i]);

    return term;
}

bool check_valid_type(Function &func, int index, Term* term)
{
    Term* expectedType = func.inputTypes[index];

    if (expectedType == ANY_TYPE)
        return true;

    return term->type == expectedType;
}

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw std::runtime_error("term is NULL");

    term->clearError();

    // Check function
    if (term->function == NULL) {
        error_occured(term, "Function is NULL");
        return;
    }

    if (!is_function(term->function)) {
        error_occured(term, "term->function is not a function");
        return;
    }

    Function& func = as_function(term->function);

    if (func.evaluate == NULL) {
        std::stringstream message;
        message << "Function '" << func.name << "' has no evaluate function";
        error_occured(term, message.str());
        return;
    }

    // Check each input. Make sure:
    //  1) it is not null
    //  2) it is up-to-date
    //  3) it has a non-null value
    //  4) it has no errors
    //  5) it has the correct type
    for (unsigned int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
    {
        int effectiveIndex = inputIndex;
        if (func.variableArgs)
            effectiveIndex = 0;

        Term* input = term->inputs[inputIndex];
        Function::InputProperties inputProps = func.getInputProperties(effectiveIndex);
         
        if (input == NULL && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " is NULL";
            error_occured(term, message.str());
            return;
        }

        if (input->value == NULL && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " has NULL value";
            error_occured(term, message.str());
            return;
        }

        if (input->hasError() && !inputProps.meta) {
            std::stringstream message;
            message << "Input " << inputIndex << " has an error";
            error_occured(term, message.str());
            return;
        }
        
        // Check type
        if (!check_valid_type(func, effectiveIndex, input)) {
            std::stringstream message;
            message << "Runtime type error: input " << inputIndex << " has type "
                << as_type(input->type).name;
            error_occured(term, message.str());
            return;
        }

        // Possibly evaluate this input if needed
        if (!inputProps.meta && input->needsUpdate) {
            assert(term != input); // prevent infinite recursion
            evaluate_term(input);
        }
    }
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (term->value == NULL) {
        alloc_value(term);
    }

    // Execute the function
    try {
        func.evaluate(term);
        term->needsUpdate = false;
    }
    catch (std::exception const& err)
    {
        error_occured(term, err.what());
    }
}

void evaluate_branch(Branch& branch, Term* errorListener)
{
    int count = branch.numTerms();
    for (int index=0; index < count; index++) {
		Term* term = branch.get(index);
        evaluate_term(term);

        if (term->hasError()) {
            std::stringstream out;
            out << "On term " << term_to_raw_string(term) << "\n" << term->getErrorMessage();
            error_occured(errorListener, out.str());
            return;
        }
    }
}

void error_occured(Term* errorTerm, std::string const& message)
{
    if (errorTerm == NULL)
        throw std::runtime_error(message);

    errorTerm->pushError(message);
}

void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);

    // Term* previousInput = term->inputs.get(index);

    term->inputs.setAt(index, input);

    // Update syntax hints
    if (input == NULL)
        term->syntaxHints.getInputSyntax(index).unknownStyle();
    else if (input->name == "")
        term->syntaxHints.getInputSyntax(index).bySource();
    else
        term->syntaxHints.getInputSyntax(index).byName(input->name);

#if TRACK_USERS
    if (input != NULL) {
        assert_good_pointer(input);
        input->users.appendUnique(term);
    }

    if (previousInput != NULL && !is_actually_using(previousInput, term))
        previousInput->users.remove(term);
#endif
}

Term* create_duplicate(Branch* branch, Term* source, bool copyBranches)
{
    Term* term = create_term(branch, source->function, source->inputs);

    term->name = source->name;

    if (copyBranches)
        copy_value(source, term);
    else
        copy_value_but_dont_copy_inner_branch(source,term);

    term->properties.import(source->properties);
    term->syntaxHints = source->syntaxHints;

    if (source->state != NULL) {
        if (copyBranches && !has_inner_branch(source))
            copy_value(source->state, term->state);

        term->state->properties.import(source->state->properties);
    }
        
    return term;
}

bool is_actually_using(Term* user, Term* usee)
{
    assert_good_pointer(user);
    assert_good_pointer(usee);

    if (user->function == usee)
        return true;

    for (unsigned int i=0; i < user->inputs.count(); i++) {
        if (user->inputs[i] == usee)
            return true;
    }

    return false;
}

Term* possibly_coerce_term(Branch* branch, Term* original, Term* expectedType)
{
    // (In the future, we will have more complicated coersion rules)
    
    // Ignore NULL
    if (original == NULL)
        return original;

    // Coerce from int to float
    if (original->type == INT_TYPE && expectedType == FLOAT_TYPE) {
        return apply_function(branch, INT_TO_FLOAT_FUNC, RefList(original));
    }

    return original;
}

Term* apply_function(Branch* branch, Term* function, RefList const& _inputs, std::string const& name)
{
    // Make a local copy of _inputs
    RefList inputs = _inputs;

    // Evaluate this function if needed
    if (function->needsUpdate)
        evaluate_term(function);

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Inputs in constructor function is not yet supported");

        function = get_value_function(function);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    else if (!is_function(function)) {

        Type& type = as_type(function->type);

        if (!type.memberFunctions.contains(""))
            throw std::runtime_error(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        RefList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        function = type.memberFunctions[""];
        inputs = memberFunctionInputs;
    }

    Function& functionData = as_function(function);

    // Possibly coerce inputs
    for (unsigned int i=0; i < inputs.count(); i++) {
        inputs.setAt(i, possibly_coerce_term(branch, inputs[i],
                functionData.inputType(i)));
    }

    // Create the term
    Term* result = create_term(branch, function, inputs);

    if (name != "" && branch != NULL)
        branch->bindName(result, name);

    return result;
}

Term* apply_function(Branch* branch,
                     std::string const& functionName, 
                     RefList const& inputs)
{
    Term* function = find_named(branch,functionName);
    if (function == NULL)
        throw std::runtime_error(std::string("function not found: ")+functionName);

    return apply_function(branch, function, inputs);
}

Term* eval_function(Branch& branch, Term* function, RefList const& inputs)
{
    Term* result = apply_function(&branch, function, inputs);
    evaluate_term(result);
    return result;
}

Term* eval_function(Branch& branch, std::string const& functionName,
        RefList const &inputs)
{
    Term* function = find_named(&branch,functionName);
    if (function == NULL)
        throw std::runtime_error(std::string("function not found: ")+functionName);

    return eval_function(branch, function, inputs);
}

void change_function(Term* term, Term* new_function)
{
    assert_good_pointer(term);

    assert_type(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good_pointer(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    assert(!map.contains(NULL));

    for (unsigned int i=0; i < term->inputs.count(); i++) {
        Term* orig = term->inputs[i];
        Term* remap = map.getRemapped(orig);
    }

    term->inputs.remapPointers(map);

    term->function = map.getRemapped(term->function);

    if ((term->value != NULL)
            && term->type != NULL
            && (as_type(term->type).remapPointers != NULL)) {

        as_type(term->type).remapPointers(term, map);
    }

    term->type = map.getRemapped(term->type);

    if (term->state != NULL)
        remap_pointers(term->state, map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_good_pointer(term);
    assert_good_pointer(original);

    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

} // namespace circa
