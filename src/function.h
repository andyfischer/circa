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
    Ref _outputType;
    Ref _hiddenStateType;

    bool _variableArgs;

    std::string _name;

    Branch parameters;

    // Code
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    ToSourceString toSourceString;

    // Associated terms
    Ref feedbackFunc;

    Function();

    void appendInput(Term* type, std::string const& name);
    void prependInput(Term* type, std::string const& name);
    InputProperties& getInputProperties(int index);
};

namespace function_t {
    void assign(Term* source, Term* dest);
    void remapPointers(Term* term, ReferenceMap const& map);
    std::string to_string(Term* term);
}

bool is_function(Term* term);
Function& as_function(Term*);

std::string get_placeholder_name_for_index(int index);

std::string& function_get_name(Term* function);
Ref& function_get_output_type(Term* function);
Ref& function_get_hidden_state_type(Term* function);
bool& function_get_variable_args(Term* function);
Term* function_get_input_type(Term* function, int index);
std::string const& function_get_input_name(Term* function, int index);
bool& function_get_input_modified(Term* function, int index);
bool& function_get_input_meta(Term* function, int index);

Function::EvaluateFunc& function_get_evaluate(Term* function);
Function::SpecializeTypeFunc& function_get_specialize_type(Term* function);
Function::ToSourceString& function_get_to_source_string(Term* function);

int function_num_inputs(Term* function);

bool is_callable(Term* term);
bool inputs_fit_function(Term* func, RefList const& inputs);
Term* create_overloaded_function(Branch* branch, std::string const& name);
Term* specialize_function(Term* func, RefList const& inputs);

} // namespace circa

#endif
