
#include "common_headers.h"
#include <string>

#include "term.h"


struct CircaFunction
{
    TermList inputTypes;
    TermList outputTypes;

    Term* stateType;

    bool pureFunction;
    bool variableArgs;

    string name;

    // Code
    void (*initialize)(Term*);
    void (*execute)(Term*);

    CircaFunction();
};

extern "C" {

CircaFunction* as_function(Term*);
void Function_alloc(Term*);
void Function_setPureFunction(Term* term, bool value);
void Function_setName(Term* term, const char* value);
void Function_setInputType(Term* term, int index, Term* type);
void Function_setOutputType(Term* term, int index, Term* type);

}
