// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "types/list.h"
#include "branch.h"
#include "term.h"

namespace circa {

struct Function
{
    typedef void (*StaticTypeQueryFunc)(StaticTypeQuery* query);
    typedef void (*PostInputChange)(Term*);
    typedef int (*GetOutputCount)(Term*);
    typedef const char* (*GetOutputName)(Term*, int index);
    typedef Type* (*GetOutputType)(Term*, int index);
    typedef void (*AssignRegisters)(Term*);
    typedef void (*PostCompile)(Term*);

    Term* declaringTerm;

    std::string name;
    List outputTypes;
    Term* implicitStateType;
    bool variableArgs;
    Term* feedbackFunc;
    TaggedValue parameter;
    bool throws;
    int outputCount;

    // Functions
    SimpleEvaluateFunc simpleEvaluate;
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

    Function();
    ~Function();
};

namespace function_attrs_t {
    void initialize(Type* type, TaggedValue* value);
    void release(Type*, TaggedValue* value);
    void copy(Type*, TaggedValue* source, TaggedValue* dest);
}

namespace function_t {

    void setup_type(Type* type);

    // accessors
    Term* get_feedback_func(Term* function);
}

bool is_function(Term* term);
bool is_function_attrs(Term* term);
Function& as_function_attrs(Term* term);
Branch* function_contents(Term* func);
Branch* function_contents(Function* func);
Function* get_function_attrs(Term* func);

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

Term* create_overloaded_function(Branch* branch, std::string const& name,
        TermList const& overloads);
Type* derive_specialized_output_type(Term* function, Term* call);

// Returns whether the given function can rebind the input at 'index'. (The
// calling code must still opt-in to this rebind.
bool function_can_rebind_input(Term* function, int index);

// Returns whether the function will implicitly rebind the input at the given
// index. (in practice, this only happens for some method calls).
bool function_implicitly_rebinds_input(Term* function, int index);

// Returns whether this term rebinds the input at 'index'
bool function_call_rebinds_input(Term* term, int index);

Type* function_get_input_type(Term* function, int index);
Type* function_get_output_type(Term* function, int index);
Type* function_get_input_type(Function* func, int index);
Type* function_get_output_type(Function* func, int index);
int function_num_inputs(Function* func);

bool function_is_state_input(Function* func, int index);
bool function_get_input_meta(Function* func, int index);
bool function_get_input_optional(Function* func, int index);

Term* function_get_input_placeholder(Function* func, int index);
Branch* function_get_contents(Function* func);
std::string function_get_input_name(Function* func, int index);

std::string function_get_documentation_string(Function* func);

const char* get_output_name(Term* term, int outputIndex);
const char* get_output_name_for_input(Term* term, int inputIndex);

// Returns whether this function is 'native', meaning that it's not a subroutine.
bool is_native_function(Function* function);

void function_set_evaluate_func(Term* func, EvaluateFunc eval);

// Change the function's EvaluateFunc, and update any terms that are using it.
void function_set_specialize_type_func(Term* func, SpecializeTypeFunc specialzeType);

void function_format_header_source(StyledSource* source, Function* func);

} // namespace circa
