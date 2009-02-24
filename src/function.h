// Copyright 2008 Andrew Fischer

#ifndef CIRCA_FUNCTION_INCLUDED
#define CIRCA_FUNCTION_INCLUDED

#include "common_headers.h"

#include "term.h"
#include "branch.h"

namespace circa {

#define INPUT_PLACEHOLDER_PREFIX "#input-"
#define OUTPUT_PLACEHOLDER_NAME "#return"

struct Function
{
    typedef void (*EvaluateFunc)(Term* caller);
    typedef PointerIterator* (*PointerIteratorFunc)(Term* caller);

    struct InputProperties {
        std::string name;
        bool modified;
        bool meta;

        InputProperties() : modified(false), meta(false) {}
    };
    typedef std::vector<InputProperties> InputPropertiesList;

    RefList inputTypes;
    InputPropertiesList inputProperties;
    Term* outputType;
    Term* stateType;

    bool pureFunction;
    bool hasSideEffects;
    bool variableArgs;

    std::string name;

    Branch subroutineBranch;

    // Code
    EvaluateFunc evaluate;
    PointerIteratorFunc startControlFlowIterator;

    // External functions
    Ref feedbackAccumulationFunction;
    Ref feedbackPropogateFunction;
    Ref generateCppFunction;
    Ref printCircaSourceFunction;

    Function();

    Term* inputType(int index);
    void appendInput(Term* type, std::string const& name);
    InputProperties& getInputProperties(unsigned int index);
    void setInputMeta(int index, bool value);
    void setInputModified(int index, bool value);

    // Hosted functions
    static void copyExceptBranch(Term* source, Term* dest);
    static void copy(Term* source, Term* dest);
    static void remapPointers(Term* term, ReferenceMap const& map);
    static void visitPointers(Term* term, PointerVisitor& visitor);
    static void subroutine_call_evaluate(Term* caller);
};

bool is_function(Term* term);
Function& as_function(Term*);

void initialize_as_subroutine(Function& func);

std::string get_placeholder_name_for_index(int index);

Branch& call_subroutine(Branch& branch, std::string const& functionName);
Branch& get_subroutine_branch(Term* term);
PointerIterator* start_function_pointer_iterator(Function* function);

// Call the start_control_flow_iterator function for this term's function,
// and return the new iterator. Returns NULL if there is no such function.
PointerIterator* start_control_flow_iterator(Term* term);

} // namespace circa

#endif
