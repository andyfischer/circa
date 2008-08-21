
#include "branch.h"
#include "evaluator.h"
#include "operations.h"
#include "subroutine.h"

namespace circa {

void
Evaluator::SubroutineScope::onClose()
{
    Subroutine_closeBranch(this->callingTerm);
}

bool
Evaluator::isFinished() const
{
    return mStack.empty();
}

void
Evaluator::evaluate(Term* term)
{
    // Special case for subroutines. Open a branch scope.
    if (is_subroutine(term->function)) {
        SubroutineScope *scope = new SubroutineScope();

        scope->callingTerm = term;
        scope->branch = Subroutine_openBranch(term);

        mStack.push(scope);

    } else {
        execute(term);
    }
}

Term*
Evaluator::getNextTerm()
{
    if (isFinished())
        return NULL;

    Scope *scope = mStack.top();
    return scope->branch->terms[scope->next];
}

void
Evaluator::runNextInstruction()
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
Evaluator::runUntilFinished()
{
    while(!isFinished()) {
        runNextInstruction();
    }
}

} // namespace circa
