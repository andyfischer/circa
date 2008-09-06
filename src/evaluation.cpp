
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
        term->eval();
    }
}

Term*
Engine::getNextTerm()
{
    if (isFinished())
        return NULL;

    Scope *scope = mStack.top();
    return scope->branch->terms[scope->next];
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

    Term* term = scope->branch->terms[scope->next];

    // Figure out what the next term is
    if ((scope->next + 1) < scope->branch->terms.count())
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

void evaluate(Term* term)
{
    if (term == NULL)
        throw errors::InternalError("term is NULL");

    if (term->function == NULL)
        throw errors::InternalError("function term is NULL");

    // Check each input. Make sure:
    //  1) they are up-to-date
    //  2) they have a non-null value
    for (int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
    {
        Term* input = term->inputs[inputIndex];

        if (input->needsUpdate)
            evaluate(input);
            
        if (input->value == NULL)
            throw errors::InternalError(string("Input named ") + input->findName() + " has NULL value.");
    }
    
    // Make sure we have an allocated value
    if (term->value == NULL) {
        // std::cout << "Reallocating term " << term->findName() << std::endl;
        as_type(term->type)->alloc(term);
    }    

    Function* func = as_function(term->function);

    if (func == NULL)
        throw errors::InternalError("function is NULL");

    if (func->evaluate == NULL) {
        std::cout << "Error: no evaluate function for " << func->name << std::endl;
        return;
    }

    try {
        func->evaluate(term);
        term->needsUpdate = false;
    }
    catch (errors::InternalError &err)
    {
        std::cout << "An internal error occured while executing " + func->name << std::endl;
        std::cout << err.message() << std::endl;
    }
}

} // namespace evaluation
} // namespace circa
