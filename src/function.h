// Copyright 2008 Andrew Fischer

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
    typedef void (*GenerateTrainingFunc)(Branch& branch, Term* subject, Term* desired);
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

    bool pureFunction;
    bool hasSideEffects;
    bool variableArgs;

    std::string name;

    Branch subroutineBranch;

    // Code
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    ReferenceIteratorFunc startControlFlowIterator;
    GenerateTrainingFunc generateTraining;
    ToSourceString toSourceString;

    // External functions
    Ref feedbackAccumulationFunction;
    Ref feedbackPropogateFunction;
    Ref generateCppFunction;

    Function();

    Term* inputType(int index);
    int numInputs();
    void appendInput(Term* type, std::string const& name);
    void prependInput(Term* type, std::string const& name);
    InputProperties& getInputProperties(unsigned int index);
    void setInputMeta(int index, bool value);
    void setInputModified(int index, bool value);

    // Hosted functions
    static void copyExceptBranch(Term* source, Term* dest);
    static void assign(Term* source, Term* dest);
    static void remapPointers(Term* term, ReferenceMap const& map);
    static void subroutine_call_evaluate(Term* caller);
    static std::string functionToSourceString(Term* source);
};

bool is_function(Term* term);
Function& as_function(Term*);

std::string get_placeholder_name_for_index(int index);

Term* create_empty_function(Branch& branch, std::string const& header);

} // namespace circa

#endif
