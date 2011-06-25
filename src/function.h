// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "types/list.h"
#include "branch.h"
#include "term.h"

namespace circa {

struct FunctionAttrs
{
    typedef void (*StaticTypeQueryFunc)(StaticTypeQuery* query);
    typedef void (*PostInputChange)(Term*);
    typedef int (*GetOutputCount)(Term*);
    typedef const char* (*GetOutputName)(Term*, int index);
    typedef Term* (*GetOutputType)(Term*, int index);
    typedef void (*AssignRegisters)(Term*);
    typedef void (*PostCompile)(Term*);

    std::string name;
    TermList outputTypes;
    Term* implicitStateType;
    bool variableArgs;
    Term* feedbackFunc;
    TaggedValue parameter;
    bool throws;
    int outputCount;

    // Functions
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    FormatSource formatSource;
    CheckInvariants checkInvariants;
    StaticTypeQueryFunc staticTypeQuery;
    PostInputChange postInputChange;
    GetOutputCount getOutputCount;
    GetOutputName getOutputName;
    GetOutputType getOutputType;
    AssignRegisters assignRegisters;
    PostCompile postCompile;

    List parameters;

    FunctionAttrs();
    ~FunctionAttrs();
};

namespace function_attrs_t {
    void initialize(Type* type, TaggedValue* value);
    void release(Type*, TaggedValue* value);
    void copy(Type*, TaggedValue* source, TaggedValue* dest);
}

namespace function_t {

    std::string to_string(Term* term);
    std::string get_header_source(Term* term);
    void format_header_source(StyledSource* source, Term* term);

    bool check_invariants(Term* term, std::string* failureMessage);

    void setup_type(Type* type);

    // accessors
    std::string const& get_name(Term* function);
    bool get_variable_args(Term* function);
    Term* get_input_placeholder(Term* function, int index);
    Term* get_input_type(Term* function, int index);
    bool is_state_input(Term* function, int index);
    std::string const& get_input_name(Term* function, int index);
    bool get_input_modified(Term* function, int index);
    bool get_input_meta(Term* function, int index);
    void set_input_meta(Term* function, int index, bool value);
    bool get_input_optional(Term* func, int index);
    std::string const& get_exposed_name_path(Term* function);
    void set_exposed_name_path(Term* func, std::string const& value);
    Term* get_feedback_func(Term* function);
    std::string get_documentation(Term* function);

    EvaluateFunc& get_evaluate(Term* function);
    SpecializeTypeFunc& get_specialize_type(Term* function);

    int num_inputs(Term* function);
}

bool is_function(Term* term);
bool is_function_attrs(Term* term);
FunctionAttrs& as_function_attrs(Term* term);
Branch& function_contents(Term* func);
FunctionAttrs* get_function_attrs(Term* func);

// Return the placeholder name for the given input index; this is the name that
// is used if no name is given.
std::string get_placeholder_name_for_index(int index);

void initialize_function(Term* func);
void finish_parsing_function_header(Term* func);

// Returns whether this term can be called as a function
bool is_callable(Term* term);

bool inputs_statically_fit_function(Term* func, TermList const& inputs);
bool inputs_fit_function_dynamic(Term* func, TermList const& inputs);
bool values_fit_function_dynamic(Term* func, List* list);

Term* create_overloaded_function(Branch& branch, std::string const& name,
        TermList const& overloads);
Term* derive_specialized_output_type(Term* function, Term* call);
void function_set_use_input_as_output(Term* function, int index, bool value);

// Returns whether the given function can rebind the input at 'index'. (The
// calling code must still opt-in to this rebind.
bool function_can_rebind_input(Term* function, int index);

// Returns whether the function will implicitly rebind the input at the given
// index. (in practice, this only happens for some member-function calls).
bool function_implicitly_rebinds_input(Term* function, int index);

// Returns whether this term rebinds the input at 'index'
bool function_call_rebinds_input(Term* term, int index);

Term* function_get_input_type(Term* function, int index);
Term* function_get_output_type(Term* function, int index);
TaggedValue* function_get_parameters(Term* function);

const char* get_output_name(Term* term, int outputIndex);
const char* get_output_name_for_input(Term* term, int inputIndex);

// Returns whether this function is 'native', meaning that it's not a subroutine.
bool is_native_function(Term* function);

} // namespace circa
