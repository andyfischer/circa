// Copyright 2008 Paul Hodge

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input-"
#define OUTPUT_PLACEHOLDER_NAME "#out"

struct Function
{
    typedef void (*EvaluateFunc)(Term* caller);
    typedef Term* (*SpecializeTypeFunc)(Term* caller);
    typedef ReferenceIterator* (*ReferenceIteratorFunc)(Term* caller);
    typedef std::string (*ToSourceString)(Term* term);

    struct InputProperties {
        std::string name;
        bool modified;
        bool meta;

        InputProperties() : modified(false), meta(false) {}
    };
    typedef std::vector<InputProperties> InputPropertiesList;

    RefList inputTypes;
    InputPropertiesList inputProperties;
    Ref outputType;
    Ref hiddenStateType;

    bool pureFunction;
    bool hasSideEffects;
    bool variableArgs;

    std::string name;

    // Code
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    ReferenceIteratorFunc startControlFlowIterator;
    ToSourceString toSourceString;

    // Associated terms
    Ref feedbackFunc;

    Function();

    Term* inputType(int index);
    std::string const& inputName(int index);
    int numInputs();
    void appendInput(Term* type, std::string const& name);
    void prependInput(Term* type, std::string const& name);
    InputProperties& getInputProperties(int index);
    void setInputMeta(int index, bool value);
    void setInputModified(int index, bool value);
};

namespace function_t {
    void assign(Term* source, Term* dest);
    void remapPointers(Term* term, ReferenceMap const& map);
    std::string to_string(Term* term);
}

bool is_function(Term* term);
Function& as_function(Term*);

std::string get_placeholder_name_for_index(int index);

bool is_callable(Term* term);
bool inputs_fit_function(Term* func, RefList const& inputs);
Term* create_overloaded_function(Branch* branch, std::string const& name);
Term* specialize_function(Term* func, RefList const& inputs);

} // namespace circa

#endif
