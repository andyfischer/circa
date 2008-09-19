// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "errors.h"
#include "evaluation.h"
#include "operations.h"
#include "subroutine.h"

namespace circa {
namespace evaluation {

void
Engine::SubroutineScope::onClose()
{
    Subroutine_closeBranch(this->callingTerm);
}

bool
Engine::isFinished() const
{
    return mStack.empty();
}

void
Engine::evaluate(Term* term)
{
    // Special case for subroutines. Open a branch scope.
    if (is_subroutine(term->function)) {
        SubroutineScope *scope = new SubroutineScope();

        scope->callingTerm = term;
        scope->branch = Subroutine_openBranch(term);

        mStack.push(scope);

    } else {
        evaluate_term(term);
    }
}

Term*
Engine::getNextTerm()
{
    if (isFinished())
        return NULL;

    Scope *scope = mStack.top();
    return scope->branch->get(scope->next);
}

void
Engine::runNextInstruction()
{
    if (isFinished())
        return;

    Scope *scope = mStack.top();

    if (mSpecialNextAction == CLOSE_BRANCH) {
        scope->onClose();  // after this call, 'branch' is invalid
        mStack.pop();
        delete scope;
        mSpecialNextAction = NONE;
        return;
    }

    Term* term = scope->branch->get(scope->next);

    // Figure out what the next term is
    if ((scope->next + 1) < scope->branch->numTerms())
        scope->next += 1;
    else
        mSpecialNextAction = CLOSE_BRANCH;

    this->evaluate(term);
}

void
Engine::runUntilFinished()
{
    while(!isFinished()) {
        runNextInstruction();
    }
}

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

    Function* func = as_function(term->function);

    if (func->evaluate == NULL) {
        std::stringstream message;
        message << "Function '" << func->name << "' has no evaluate function";
        term->pushError(message.str());
        return;
    }

    // Check each input. Make sure:
    //  1) they are not null
    //  1) they are up-to-date
    //  2) they have a non-null value
    for (int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
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

        if (input->needsUpdate)
            evaluate_term(input);
    }
    
    // Make sure we have an allocated value
    if (term->value == NULL) {
        // std::cout << "Reallocating term " << term->findName() << std::endl;
        as_type(term->type)->alloc(term);
    }    

    try {
        func->evaluate(term);
        term->needsUpdate = false;
    }
    catch (errors::InternalError &err)
    {
        std::stringstream message;
        message << "An internal error occured while executing " + func->name << ": "
            << err.message();
        term->pushError(message.str());
    }
}

} // namespace evaluation
} // namespace circa
