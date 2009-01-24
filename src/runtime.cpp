// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "compilation.h"
#include "debug.h"
#include "introspection.h"
#include "runtime.h"
#include "function.h"
#include "parser.h"
#include "type.h"
#include "values.h"

namespace circa {

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
         
        if (input == NULL) {
            std::stringstream message;
            message << "Input " << inputIndex << " is NULL";
            error_occured(term, message.str());
            return;
        }

        if (input->value == NULL) {
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
        std::stringstream message;
        message << "An exception occured while executing " + func.name << ": "
            << err.what();
        error_occured(term, message.str());
    }
}

void evaluate_branch(Branch& branch, Term* errorListener)
{
    int count = branch.numTerms();
    for (int index=0; index < count; index++) {
		Term* term = branch.get(index);
        evaluate_term(term);

        if (term->hasError()) {
            error_occured(errorListener, term->getErrorMessage());
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

Term* create_term(Branch* branch, Term* function, ReferenceList const& inputs)
{
    assert_good_pointer(function);

    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    term->function = function;
    function->users.appendUnique(term);

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

    // Run the function's initialize (if it has one)
    if (functionData.initialize != NULL) {
        functionData.initialize(term);
    }

    return term;
}

void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);

    Term* previousInput = term->inputs.get(index);

    term->inputs.setAt(index, input);

    if (input != NULL) {
        assert_good_pointer(input);
        input->users.appendUnique(term);
    }

    if (previousInput != NULL && !is_actually_using(previousInput, term))
        previousInput->users.remove(term);
}

void set_inputs(Term* term, ReferenceList inputs)
{
    ReferenceList previousInputs = term->inputs;

    term->inputs = inputs;

    for (int i=0; i < (int) inputs.count(); i++) {
        if (inputs[i] == NULL)
            continue;

        inputs[i]->users.appendUnique(term);
    }

    for (int i=0; i < (int) previousInputs.count(); i++) {
        if (previousInputs[i] == NULL)
            continue;
        if (!is_actually_using(previousInputs[i], term))
            previousInputs[i]->users.remove(term);
    }
}

void delete_term(Term* term)
{
    if (term->state != NULL)
        delete_term(term->state);
    term->state = NULL;
    dealloc_value(term);

#if DEBUG_CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.erase(term);
#endif

#if !DEBUG_NEVER_DELETE_TERMS
    delete term;
#endif
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

Term* possibly_coerce_term(Branch& branch, Term* original, Term* expectedType)
{
    // (In the future, we will have more complicated coersion rules)
    
    // Ignore NULL
    if (original == NULL)
        return original;

    // Coerce from int to float
    if (original->type == INT_TYPE && expectedType == FLOAT_TYPE) {
        return apply_function(branch, INT_TO_FLOAT_FUNC, ReferenceList(original));
    }

    return original;
}

Term* apply_function(Branch& branch, Term* function, ReferenceList const& _inputs)
{
    // Make a local copy of _inputs
    ReferenceList inputs = _inputs;

    // Evaluate this function if needed
    if (function->needsUpdate)
        evaluate_term(function);

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Multiple inputs in constructor not supported");

        function = get_value_function(branch, function);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    else if (!is_function(function)) {

        Type& type = as_type(function->type);

        if (!type.memberFunctions.contains(""))
            throw std::runtime_error(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        ReferenceList memberFunctionInputs;
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

    // Attempt to re-use an existing term
    // Disabled because sometimes the contents of 'users' is bad (FIXME)
    Term* existing = NULL; // find_equivalent(function, inputs);

    if (existing != NULL)
        return existing;

    // Create the term
    return create_term(&branch, function, inputs);
}

Term* apply_function(Branch& branch,
                     std::string const& functionName, 
                     ReferenceList const& inputs)
{
    Term* function = branch.findNamed(functionName);
    if (function == NULL)
        throw std::runtime_error(std::string("function not found: ")+functionName);

    return apply_function(branch, function, inputs);
}

Term* eval_function(Branch& branch, Term* function, ReferenceList const& inputs)
{
    Term* result = apply_function(branch, function, inputs);
    evaluate_term(result);
    return result;
}

Term* eval_function(Branch& branch, std::string const& functionName,
        ReferenceList const &inputs)
{
    Term* function = branch.findNamed(functionName);
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

    term->inputs.remapPointers(map);

    term->function = map.getRemapped(term->function);

    if ((term->value != NULL)
            && term->type != NULL
            && (as_type(term->type).remapPointers != NULL))
        as_type(term->type).remapPointers(term, map);

    term->type = map.getRemapped(term->type);
}

void visit_pointers(Term* term, PointerVisitor &visitor)
{
    Type& type = as_type(term->type);

    term->inputs.visitPointers(visitor);
    visitor.visitPointer(term->function);
    visitor.visitPointer(term->type);

    if (type.visitPointers != NULL)
        type.visitPointers(term, visitor);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_good_pointer(term);
    assert_good_pointer(original);

    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void can_safely_point_to(Term* pointer, Term* pointee)
{

}

} // namespace circa
