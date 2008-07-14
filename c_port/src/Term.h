
#include <vector>

#include "TermSyntaxHints.h"

class Branch;
class CircaObject;

struct Term
{
    Branch* owningBranch;
    std::vector<Term*> inputs;
    Term* function;
    std::vector<Term*> users;

    CircaObject* outputValue;
    CircaObject* state;

    bool needsUpdate;

    TermSyntaxHints syntaxHints;

    int globaID;
};

Term* Term_createRaw();
Term* Term_getInput(Term* term, int index);
Term* Term_getFunction(Term* term);
CircaObject* Term_getOutputValue(Term* term);
CircaObject* Term_getState(Term* term);

void ForeignFunctionTest();
