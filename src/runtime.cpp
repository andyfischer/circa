// Copyright 2008 Paul Hodge

#include "branch.h"
#include "errors.h"
#include "runtime.h"
#include "function.h"
#include "parser.h"
#include "type.h"
#include "values.h"

namespace circa {

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw std::runtime_error("term is NULL");

    term->clearErrors();

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
    //  1) they are not null
    //  2) they are up-to-date
    //  3) they have a non-null value
    //  4) they have no errors
    for (unsigned int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
    {
        Term* input = term->inputs[inputIndex];
         
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

        if (input->hasError()) {
            std::stringstream message;
            message << "Input " << inputIndex << " has an error";
            error_occured(term, message.str());
            return;
        }

        if (input->needsUpdate)
            evaluate_term(input);
    }
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (term->value == NULL) {
        as_type(term->type)->alloc(term);
    }    

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

void evaluate_branch(Branch& branch)
{
    int count = branch.numTerms();
    for (int index=0; index < count; index++) {
		Term* term = branch.get(index);
        evaluate_term(term);
    }
}

Branch* evaluate_file(std::string const& filename)
{
    Branch *branch = new Branch();

    Branch temp_branch;
    temp_branch.bindName(string_var(temp_branch, filename), "filename");
    std::string file_contents = as_string(eval_statement(temp_branch,
                "read-text-file(filename)"));

    token_stream::TokenStream tokens(file_contents);
    ast::StatementList *statementList = parser::statementList(tokens);

    statementList->createTerms(*branch);

    delete statementList;

    return branch;
}

void error_occured(Term* errorTerm, std::string const& message)
{
    //std::cout << "error occured: " << message << std::endl;
    errorTerm->pushError(message);
}

Term* create_term(Branch* branch, Term* function, ReferenceList inputs)
{
    //if (branch == NULL)
    //    throw std::runtime_error("in create_term, branch is NULL");
    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    term->function = function;

    // Add to users of function
    function->users.appendUnique(term);
    
    Function& functionData = as_function(function);

    Term* outputType = functionData.outputType;
    Term* stateType = functionData.stateType;

    if (outputType == NULL)
        throw std::runtime_error("outputType is NULL");
        
    if (!is_type(outputType))
        throw std::runtime_error(outputType->findName() + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw std::runtime_error(outputType->findName() + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) {
        term->state = create_var(NULL, stateType);
    }
    else
        term->state = NULL;

    set_inputs(term, inputs);

    // Run the function's initialize (if it has one)
    if (functionData.initialize != NULL) {
        functionData.initialize(term);
    }

    return term;
}

void set_inputs(Term* term, ReferenceList inputs)
{
    assert_good(term);

    term->inputs = inputs;

    for (unsigned int i=0; i < inputs.count(); i++)
    {
        Term* inputTerm = inputs[i];
        if (inputTerm != NULL) {
            assert_good(inputTerm);
            inputTerm->users.appendUnique(term);
        }
    }
}

void set_input(Term* term, int index, Term* input)
{
    assert_good(term);

    term->inputs.setAt(index, input);

    if (input != NULL) {
        assert_good(input);
        input->users.appendUnique(term);
    }
}

Term* create_var(Branch* branch, Term* type)
{
    Term *term = create_term(branch, get_var_function(*branch, type), ReferenceList());
    term->stealingOk = false;
    return term;
}

Term* apply_function(Branch& branch, Term* function, ReferenceList inputs)
{
    if (function->needsUpdate)
        function->eval();

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Multiple inputs in constructor not supported");

        function = get_var_function(branch, function);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    else if (!is_function(function)) {

        Type* type = as_type(function->type);

        if (!type->memberFunctions.contains(""))
            throw std::runtime_error(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        ReferenceList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        function = type->memberFunctions[""];
        inputs = memberFunctionInputs;
    }

    // Create the term
    return create_term(&branch, function, inputs);
}

Term* eval_function(Branch& branch, Term* function, ReferenceList inputs)
{
    Term* result = apply_function(branch, function, inputs);
    result->eval();
    return result;
}

Term* eval_function(Branch& branch, std::string const& functionName, ReferenceList inputs)
{
    Term* function = branch.findNamed(functionName);
    if (function == NULL)
        throw std::runtime_error(std::string("function not found: ")+functionName);

    return eval_function(branch, function, inputs);
}

void change_function(Term* term, Term* new_function)
{
    assert_good(term);

    assert_type(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good(term);

    term->inputs.remapPointers(map);
    term->function = map.getRemapped(term->function);

    if (as_type(term->type)->remapPointers != NULL)
        as_type(term->type)->remapPointers(term, map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_good(term);
    assert_good(original);

    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

bool is_equivalent(Term* target, Term* function, ReferenceList const& inputs)
{
    // If this function has state, then always return false
    if (as_function(function).stateType != VOID_TYPE)
        return false;

    // Check inputs
    unsigned int numInputs = target->inputs.count();

    if (numInputs != inputs.count())
        return false;

    for (unsigned int i=0; i < numInputs; i++) {
        if (target->inputs[i] != inputs[i])
            return false;
    }

    return true;
}

Term* find_equivalent_existing(Term* function, ReferenceList const& inputs)
{
    for (unsigned int inputIndex=0; inputIndex < inputs.count(); inputIndex++) {
        Term* input = inputs[inputIndex];

        if (input == NULL)
            continue;

        unsigned int numUsers = input->users.count();
        for (unsigned int userIndex=0; userIndex < numUsers; userIndex++) {
            Term* user = input->users[userIndex];

            assert_good(user);

            if (is_equivalent(user, function, inputs))
                return user;
        }
    }

    return NULL;
}

} // namespace circa
