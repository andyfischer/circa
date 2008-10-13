// Copyright 2008 Paul Hodge

#include "branch.h"
#include "errors.h"
#include "runtime.h"
#include "function.h"
#include "operations.h"
#include "parser.h"
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
        error_occured(term, "is_function(term->function) is false");
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
    
    // Make sure we have an allocated value
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

    statementList->createTerms(branch);

    delete statementList;

    return branch;
}

void error_occured(Term* errorTerm, std::string const& message)
{
    //std::cout << "error occured: " << message << std::endl;
    errorTerm->pushError(message);
}

} // namespace circa
