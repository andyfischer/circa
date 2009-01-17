// Copyright 2008 Paul Hodge

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input-"
#define OUTPUT_PLACEHOLDER_NAME "#output"

struct Function
{
    typedef void (*InitializeFunc)(Term* caller);
    typedef void (*EvaluateFunc)(Term* caller);

    struct InputProperties {
        bool modified;
        bool meta;

        InputProperties() : modified(false), meta(false) {}
    };
    typedef std::vector<InputProperties> InputPropertiesList;

    ReferenceList inputTypes;
    InputPropertiesList inputProperties;
    Term* outputType;
    Term* stateType;

    bool pureFunction;
    bool hasSideEffects;
    bool variableArgs;

    std::string name;

    Branch subroutineBranch;

    // Code
    InitializeFunc initialize;
    EvaluateFunc evaluate;

    // External functions
    Term* feedbackAccumulationFunction;
    Term* feedbackPropogateFunction;
    Term* generateCppFunction;
    Term* printCircaSourceFunction;

    Function();

    Term* inputType(int index);
    InputProperties& getInputProperties(unsigned int index);
    void setInputMeta(int index, bool value);
    void setInputModified(int index, bool value);

    // Hosted functions
    static void duplicate(Term* source, Term* dest);
    static void remapPointers(Term* term, ReferenceMap const& map);
    static void visitPointers(Term* term, PointerVisitor& visitor);
    static void subroutine_evaluate(Term* caller);
};

bool is_function(Term* term);
Function& as_function(Term*);

std::string get_placeholder_name_for_index(int index);

Branch& call_subroutine(Branch& branch, std::string const& functionName);

} // namespace circa

#endif
