// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "operations.h"
#include "parser.h"

namespace circa {

void evaluate_term(Term* term)
{
    if (term == NULL)
        throw errors::InternalError("term is NULL");

    term->clearErrors();

    // Check function
    if (term->function == NULL) {
        term->pushError("Function is NULL");
        return;
    }

    if (!is_function(term->function)) {
        term->pushError("is_function(term->function) is false");
        return;
    }

    Function& func = as_function(term->function);

    if (func.evaluate == NULL) {
        std::stringstream message;
        message << "Function '" << func.name << "' has no evaluate function";
        term->pushError(message.str());
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
            term->pushError(message.str());
            return;
        }

        if (input->value == NULL) {
            std::stringstream message;
            message << "Input " << inputIndex << " has NULL value";
            term->pushError(message.str());
            return;
        }

        if (input->hasError()) {
            std::stringstream message;
            message << "Input " << inputIndex << " has an error";
            term->pushError(message.str());
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
    catch (errors::InternalError &err)
    {
        std::stringstream message;
        message << "An internal error occured while executing " + func.name << ": "
            << err.message();
        term->pushError(message.str());
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
    temp_branch.bindName(constant_string(&temp_branch, filename), "filename");
    std::string file_contents = as_string(eval_statement(temp_branch,
                "read-text-file(filename)"));

    token_stream::TokenStream tokens(file_contents);
    ast::StatementList *statementList = parser::statementList(tokens);

    statementList->createTerms(branch);

    delete statementList;

    return branch;
}

} // namespace circa
