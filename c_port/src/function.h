#ifndef CIRCA__FUNCTION__INCLUDED
#define CIRCA__FUNCTION__INCLUDED

#include "common_headers.h"
#include <string>

#include "term.h"

struct Function
{
    typedef void (*ExecuteFunc)(Term caller);

    TermList inputTypes;
    Term outputType;

    Term stateType;

    // recycleInput is an index of which input we want to recycle.
    // This means that when executing, the runtime will do one of
    // two things:
    //  1) the runtime may copy this input to the calling term
    //  2) or, 'steal' the value from this input and give it to
    //     the calling term (this is much more efficient).
    // Either way, the execute function should not attempt to access
    // the value of the input that it wants to recycle.
    // This value may also be -1, which means to not try to recycle.
    // -1 is the default.
    int recycleInput;

    bool pureFunction;
    bool variableArgs;

    string name;

    // Code
    void (*initialize)(Term);
    ExecuteFunc execute;

    Function();
};


void Function_alloc(Term caller);
bool is_function(Term term);
Function* as_function(Term);

void initialize_functions(Branch* kernel);

#endif
