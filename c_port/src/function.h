
#include "common_headers.h"
#include <string>

#include "object.h"
#include "term.h"
#include "exec_context.h"

#define CA_FUNCTION(t) ((CircaFunction*)t->outputValue)

struct CircaFunction : public CircaObject
{
    TermList inputTypes;
    TermList outputTypes;

    Term* stateType;

    bool pureFunction;
    bool variableArgs;

    string name;

    // Code
    void (*initialize)(Term*);
    void (*execute)(ExecContext*);

    CircaFunction();
};

extern "C" {

CircaObject* CaFunction_alloc(Term*);
void CaFunction_setPureFunction(Term* term, bool value);
void CaFunction_setName(Term* term, const char* value);
void CaFunction_setInputType(Term* term, int index, Term* type);
void CaFunction_setOutputType(Term* term, int index, Term* type);

}
