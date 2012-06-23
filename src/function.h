// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "gc.h"
#include "term.h"

namespace circa {

struct Function
{
    typedef void (*StaticTypeQueryFunc)(StaticTypeQuery* query);
    typedef void (*OnCreateCall)(Term*);
    typedef void (*PostInputChange)(Term*);
    typedef int (*GetOutputCount)(Term*);
    typedef const char* (*GetOutputName)(Term*, int index);
    typedef Type* (*GetOutputType)(Term*, int index);
    typedef void (*AssignRegisters)(Term*);
    typedef void (*PostCompile)(Term*);

    CircaObject header;

    Term* declaringTerm;

    Branch* contents;

    std::string name;
    Term* feedbackFunc;
    Value parameter;
    bool throws;

    // Functions
    EvaluateFunc evaluate;
    
    SpecializeTypeFunc specializeType;
    FormatSource formatSource;
    CheckInvariants checkInvariants;
    StaticTypeQueryFunc staticTypeQuery;
    OnCreateCall onCreateCall;
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

namespace function_t {
    void initialize(Type* type, caValue* value);
    void release(Type*, caValue* value);
    void copy(Type*, caValue* source, caValue* dest);

    void setup_type(Type* type);
}

bool is_function_attrs(Term* term);
Branch* function_contents(Term* func);
Branch* function_contents(Function* func);

// Return the placeholder name for the given input index; this is the name that
// is used if no name is given.
std::string get_placeholder_name_for_index(int index);

// Create a declared function
Term* create_function(Branch* branch, const char* name);

void initialize_function(Term* func);
void finish_building_function(Branch* func);

Type* derive_specialized_output_type(Term* function, Term* call);

// Returns whether this term rebinds the input at 'index'
bool function_call_rebinds_input(Term* term, int index);

Type* function_get_input_type(Term* function, int index);
Type* function_get_output_type(Term* function, int index);
Type* function_get_input_type(Function* func, int index);
Type* function_get_output_type(Function* func, int index);

// Returns the number of input placeholders. Note that a function might have one
// input placeholder, but accept a variable number of inputs.
int function_num_inputs(Function* func);
int function_num_outputs(Function* func);

// Stateful inputs
bool function_is_state_input(Function* func, int index);
bool function_has_state_input(Function* func);

bool function_get_input_meta(Function* func, int index);
bool function_has_variable_args(Function* func);
bool function_has_variable_args(Term* func);

Term* function_get_input_placeholder(Function* func, int index);
Term* function_get_output_placeholder(Function* func, int index);

Branch* function_get_contents(Function* func);
std::string function_get_input_name(Function* func, int index);

bool function_input_is_extra_output(Function* func, int index);
std::string function_get_documentation_string(Function* func);

const char* get_output_name(Term* term, int outputIndex);

void function_set_evaluate_func(Term* func, EvaluateFunc eval);

// Change the function's EvaluateFunc, and update any terms that are using it.
void function_set_specialize_type_func(Term* func, SpecializeTypeFunc specialzeType);

void function_format_header_source(caValue* source, Branch* func);
void function_format_source(caValue* source, Term* term);

void function_set_empty_evaluation(Function* function);

void evaluate_subroutine(caStack*);
bool is_subroutine(Term* term);
bool is_subroutine(Branch* branch);

// Perform various steps to finish creating a subroutine
void initialize_subroutine(Term* sub);

} // namespace circa
