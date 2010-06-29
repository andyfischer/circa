// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#define CA_START_FUNCTIONS \
    struct _circa_StaticFuncDeclaration; \
    std::vector<_circa_StaticFuncDeclaration*> _circa_START_FUNCTIONS; \
    struct _circa_StaticFuncDeclaration { \
        const char* _header; \
        circa::EvaluateFunc _func; \
        _circa_StaticFuncDeclaration(const char* header, circa::EvaluateFunc func) \
            : _header(header), _func(func) \
        { _circa_START_FUNCTIONS.push_back(this); } \
    };


#ifdef NEW_EVALUATE

#define CA_FUNCTION(fname) \
    void fname(EvalContext* _circa_cxt, Term* _circa_caller, Term* circa_func, \
            RefList const& _circa_inputs, TaggedValue* _circa_output)

#define CA_DEFINE_FUNCTION(fname, header) \
    void evaluate_##fname(EvalContext* _circa_cxt, Term* _circa_caller); \
    static _circa_StaticFuncDeclaration _static_decl_for_##fname(header, evaluate_##fname); \
    void evaluate_##fname(EvalContext* _circa_cxt, Term* _circa_caller)

#define INPUT(index) ((TaggedValue*) _circa_inputs[index])
#define FLOAT_INPUT(index) as_float(INPUT(index))
#define BOOL_INPUT(index) as_bool(INPUT(index))
#define STRING_INPUT(index) as_string(INPUT(index))
#define INT_INPUT(index) as_int(INPUT(index))
#define NUM_INPUTS (_circa_inputs->length())
#define INPUT_TERM(index) (_circa_caller->input(index))
#define OUTPUT ((TaggedValue*) _circa_output)
#define CONTEXT (_circa_cxt)
#define FUNCTION (_circa_func)
#define CALLER (_circa_caller)
#define CONTEXT_AND_CALLER _circa_cxt, _circa_caller

#else

#define CA_FUNCTION(fname) \
    void fname(EvalContext* _circa_cxt, Term* _circa_caller)

#define CA_DEFINE_FUNCTION(fname, header) \
    CA_FUNCTION(evaluate_##fname); \
    static _circa_StaticFuncDeclaration _static_decl_for_##fname(header, evaluate_##fname); \
    CA_FUNCTION(evaluate_##fname)
 
#define INT_INPUT(index) int_input(_circa_caller, index)
#define FLOAT_INPUT(index) float_input(_circa_caller, index)
#define BOOL_INPUT(index) bool_input(_circa_caller, index)
#define STRING_INPUT(index) string_input(_circa_caller, index)
#define INPUT(index) ((TaggedValue*) _circa_caller->input(index))
#define NUM_INPUTS (_circa_caller->numInputs())
#define INPUT_TERM(index) (_circa_caller->input(index))
#define OUTPUT ((TaggedValue*) _circa_caller)
#define CONTEXT (_circa_cxt)
#define FUNCTION (_circa_caller->function)
#define CALLER (_circa_caller)
#define CONTEXT_AND_CALLER _circa_cxt, _circa_caller

#endif

#define CA_SETUP_FUNCTIONS(branch) {\
    for (size_t i=0; i < _circa_START_FUNCTIONS.size(); i++) \
        import_function(branch, _circa_START_FUNCTIONS[i]->_func,\
                _circa_START_FUNCTIONS[i]->_header);\
    }
