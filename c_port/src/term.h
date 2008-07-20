#ifndef __TERM_INCLUDED__
#define __TERM_INCLUDED__

#include <vector>

class Branch;
class CircaInt;
class CircaFloat;
class CircaBool;
class CircaFunction;
class CircaString;
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

    // Convenience constructors
    TermList(Term* term) {
        items.push_back(term);
    }
    TermList(Term* term1, Term* term2) {
        items.push_back(term1);
        items.push_back(term2);
    }
    TermList(Term* term1, Term* term2, Term* term3) {
        items.push_back(term1);
        items.push_back(term2);
        items.push_back(term3);
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

    void* value;
    Term* type;

    Term* state;

    bool needsUpdate;

    TermSyntaxHints syntaxHints;

    int globaID;

    Term() {}
    void execute();
};

int& as_int(Term* t);
float& as_float(Term* t);
bool& as_bool(Term* t);
string& as_string(Term* t);


extern "C" {

Term* Term_getInput(Term* term, int index);
Term* Term_getFunction(Term* term);

}


#endif
