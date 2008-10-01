#ifndef CIRCA__FUNCTION__INCLUDED
#define CIRCA__FUNCTION__INCLUDED

#include "common_headers.h"

#include "term.h"

namespace circa {

struct Function
{
    typedef void (*InitializeFunc)(Term* caller);
    typedef void (*EvaluateFunc)(Term* caller);

    ReferenceList inputTypes;
    Term* outputType;

    Term* stateType;

    bool pureFunction;
    bool variableArgs;

    string name;

    // Code
    InitializeFunc initialize;
    EvaluateFunc evaluate;

    Term* feedbackAccumulationFunction;
    Term* feedbackPropagationFunction;

    Function();
};


void Function_alloc(Term* caller);
void Function_dealloc(Term* caller);
void Function_duplicate(Term* source, Term* dest);
bool is_function(Term* term);
Function* as_function(Term*);

void initialize_functions(Branch* kernel);

} // namespace circa

#endif
