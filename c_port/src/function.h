
#include "common_headers.h"
#include <string>

#include "term.h"


struct Function
{
    TermList inputTypes;
    Term* outputType;

    Term* stateType;

    bool pureFunction;
    bool variableArgs;

    string name;

    // Code
    void (*initialize)(Term*);
    void (*execute)(Term*);

    Function();
};

extern "C" {

Function* as_function(Term*);
void Function_alloc(Term*);
void Function_setPureFunction(Term* term, bool value);
void Function_setName(Term* term, const char* value);
void Function_setInputType(Term* term, int index, Term* type);
void Function_setOutputType(Term* term, Term* type);
void Function_setExecute(Term* term, void(*executeFunc)(Term*));

}
