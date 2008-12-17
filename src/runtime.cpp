// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "compilation.h"
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

        if (input->needsUpdate) {
            assert(term != input); // prevent infinite recursion
            evaluate_term(input);
        }
    }
    
    // Make sure we have an allocated value. Allocate one if necessary
    if (term->value == NULL) {
        alloc_value(term);
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

Branch* evaluate_file(std::string const& filename)
{
    Branch *branch = new Branch();

    Branch temp_branch;
    temp_branch.bindName(string_var(temp_branch, filename), "filename");
    std::string file_contents = as_string(eval_statement(temp_branch,
                "read-text-file(filename)"));

    token_stream::TokenStream tokens(file_contents);
    ast::StatementList *statementList = parser::statementList(tokens);

    CompilationContext context;
    context.push(branch, NULL);
    statementList->createTerms(context);

    delete statementList;

    return branch;
}

void error_occured(Term* errorTerm, std::string const& message)
{
    if (errorTerm == NULL)
        throw std::runtime_error(message);

    errorTerm->pushError(message);
}

Term* create_term(Branch* branch, Term* function, ReferenceList const& inputs)
{
    //if (branch == NULL)
    //    throw std::runtime_error("in create_term, branch is NULL");
    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    term->function = function;

    Function& functionData = as_function(function);

    Term* outputType = functionData.outputType;

    Term* stateType = functionData.stateType;

    if (outputType == NULL)
        throw std::runtime_error("outputType is NULL");
        
    if (!is_type(outputType))
        throw std::runtime_error(outputType->name + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw std::runtime_error(outputType->name + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) {
        term->state = create_var(term->getMyBranch(), stateType);
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

void set_inputs(Term* term, ReferenceList const& inputs)
{
    assert_good_pointer(term);

    term->inputs = inputs;
}

void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);

    term->inputs.setAt(index, input);
}

Term* create_var(Branch* branch, Term* type)
{
    Term *var_function = get_var_function(*branch, type);
    Term *term = create_term(branch, var_function, ReferenceList());
    alloc_value(term);
    term->stealingOk = false;
    return term;
}

Term* apply_function(Branch& branch, Term* function, ReferenceList const& _inputs)
{
    // Make a local copy of _inputs
    ReferenceList inputs = _inputs;

    if (function->needsUpdate)
        evaluate_term(function);

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Multiple inputs in constructor not supported");

        function = get_var_function(branch, function);
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

    // Attempt to re-use an existing term
    Term* existing = find_equivalent(function, inputs);

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

    if ((term->value != NULL) && (as_type(term->type).remapPointers != NULL))
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

void assign_pointer(Term*& pointer, Term* value, Term* owner)
{
    assert(owner != NULL);

    Term* previousValue = pointer;

    if (pointer == value)
        // noop
        return;

    pointer = value;

    if (previousValue != NULL) {
        // check to remove 'owner' from users of 'previousValue'
        struct DoesTermPointToThis : public PointerVisitor
        {
            Term* target;
            bool answer;
            DoesTermPointToThis(Term* _target) : target(_target), answer(false) {}

        };

        // todo..
    }

    if (pointer != NULL) {
        pointer->users.add(owner);
    }
}

} // namespace circa
