#ifndef CIRCA__EVALUATOR__INCLUDED
#define CIRCA__EVALUATOR__INCLUDED

#include "common_headers.h"

namespace circa {
namespace evaluation {

class Engine {

    struct Scope {
        Term* callingTerm;
        Branch* branch;
        int next;

        Scope() : callingTerm(NULL), branch(NULL), next(0) {}
        virtual void onClose() {}
    };

    enum SpecialAction {
        NONE = 0,
        CLOSE_BRANCH
    };

    struct SubroutineScope : public Scope {
        virtual void onClose();
    };

    std::stack<Scope*> mStack;
    SpecialAction mSpecialNextAction;

public:
    bool isFinished() const;
    void evaluate(Term* term);
    Term* getNextTerm();
    void runNextInstruction();
    void runUntilFinished();
};

void evaluate(Term* term);

}
}

#endif
