
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

void
Evaluator::runNextInstruction()
{
    // Don't do anything if stack is empty
    if (mStack.empty())
        return;

    Scope *top = mStack.top();

    // Check if we have finished this branch
    if (top->next >= top->branch->terms.count()) {

        top->onClose();  // after this call, 'branch' is invalid
        mStack.pop();
        delete top;
        return;
    }

    Term* term = top->branch->terms[top->next];
    top->next += 1;

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
