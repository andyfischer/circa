// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "evaluation.h"

#define CA_START_FUNCTIONS \
    struct _circa_StaticFuncDeclaration; \
    static std::vector<_circa_StaticFuncDeclaration*> _circa_START_FUNCTIONS; \
    struct _circa_StaticFuncDeclaration { \
        const char* _header; \
        circa::EvaluateFunc _func; \
        _circa_StaticFuncDeclaration(const char* header, circa::EvaluateFunc func) \
            : _header(header), _func(func) \
        { _circa_START_FUNCTIONS.push_back(this); } \
    };


#define CA_DEFINE_FUNCTION(fname, header) \
    CA_FUNCTION(evaluate_##fname); \
    static _circa_StaticFuncDeclaration _static_decl_for_##fname(header, evaluate_##fname); \
    CA_FUNCTION(evaluate_##fname)

#define CA_SETUP_FUNCTIONS(branch) {\
    for (size_t i=0; i < _circa_START_FUNCTIONS.size(); i++) \
        import_function(branch, _circa_START_FUNCTIONS[i]->_func,\
                _circa_START_FUNCTIONS[i]->_header);\
    }

#define CALLER ((Term*) circa_caller_term(_stack))
#define CONTEXT ((EvalContext*) _stack)
#define STACK (_stack)
#define NUM_INPUTS (circa_num_inputs(_stack))
#define INPUT(index) (circa_input(_stack, (index)))
#define INPUT_TERM(index) ((Term*) circa_caller_input_term(_stack, (index)))
#define FLOAT_INPUT(index) (circa_float_input(_stack, (index)))
#define BOOL_INPUT(index) (circa_bool_input(_stack, (index)))
#define STRING_INPUT(index) (circa_string_input(_stack, (index)))
#define INT_INPUT(index) (circa_int_input(_stack, (index)))
#define OUTPUT (circa_output(_stack, 0))
#define OUTPUT_NTH(index) (circa_output(_stack, (index)))
#define EXTRA_OUTPUT(index) (circa_output(_stack, 1 + (index)))
#define RAISE_ERROR(msg) (circa_output_error(_stack, (msg)))
#define CONSUME_INPUT(index, out) (circa_copy(circa_input(_stack, (index)), (out)))
