#ifndef __TERM_INCLUDED__
#define __TERM_INCLUDED__

#include <vector>

class Branch;
class CircaInt;
class CircaFloat;
class CircaBool;
class CircaFunction;
class CircaString;
class CircaObject;
class CircaType;
class Term;

struct TermSyntaxHints
{
    // Todo
};

struct TermList
{
    std::vector<Term*> items;

    TermList() { }

    // Convenience constructor
    TermList(Term* term) {
        items.push_back(term);
    }

    void setAt(int index, Term* term);
    Term* operator[](int index);
};

struct Term
{
    Branch* owningBranch;
    TermList inputs;
    Term* function;
    std::vector<Term*> users;

    CircaObject* outputValue;
    CircaObject* state;

    bool needsUpdate;

    TermSyntaxHints syntaxHints;

    int globaID;

    Term() {}
    Term* getType() const;
    void execute();

    CircaInt* asInt();
    CircaFloat* asFloat();
    CircaBool* asBool();
    CircaString* asString();
    CircaFunction* asFunction();
    CircaType* asType();
};


extern "C" {

Term* Term_getInput(Term* term, int index);
Term* Term_getFunction(Term* term);
CircaObject* Term_getOutputValue(Term* term);
CircaObject* Term_getState(Term* term);

}


#endif
