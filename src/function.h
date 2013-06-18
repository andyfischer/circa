// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "block.h"
#include "object.h"
#include "term.h"

namespace circa {

struct Function
{
    typedef void (*OnCreateCall)(Term*);
    typedef void (*PostInputChange)(Term*);
    typedef void (*PostCompile)(Term*);

    CircaObject header;

    Term* declaringTerm;

    Block* contents;

    std::string name;
    Term* feedbackFunc;
    Value parameter;
    bool throws;

    // Functions
    FormatSource formatSource;
    CheckInvariants checkInvariants;
    OnCreateCall onCreateCall;
    PostInputChange postInputChange;
    PostCompile postCompile;

    List parameters;

    Function();
    ~Function();
};

namespace function_t {
    void copy(Type*, caValue* source, caValue* dest);

    void setup_type(Type* type);
}

bool is_function_attrs(Term* term);
Block* function_contents(Term* func);
Block* function_contents(Function* func);
Function* get_function_from_block(Block* block);

// Create a declared function
Term* create_function(Block* block, const char* name);

void initialize_function(Term* func);
void finish_building_function(Block* func);

Type* derive_specialized_output_type(Term* function, Term* call);

// Returns whether this term rebinds the input at 'index'
bool function_call_rebinds_input(Term* term, int index);

Block* function_get_contents(Function* func);
std::string function_get_input_name(Function* func, int index);

bool function_input_is_extra_output(Function* func, int index);
std::string function_get_documentation_string(Function* func);

const char* get_output_name(Term* term, int outputIndex);

void function_format_header_source(caValue* source, Block* func);
void function_format_source(caValue* source, Term* term);

void evaluate_subroutine(caStack*);
bool is_subroutine(Term* term);
bool is_subroutine(Block* block);

// Perform various steps to finish creating a subroutine
void initialize_subroutine(Term* sub);

} // namespace circa
