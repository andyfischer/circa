// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

#define CALLER (current_term(_cxt))
#define CONTEXT (_cxt)
#define NUM_INPUTS (_ninputs)
#define INPUT(index) (_vals[index])
#define INPUTS (_vals)
#define INPUT_TERM(index) (CALLER->input(index))
#define FLOAT_INPUT(index) (circa::to_float(INPUT(index)))
#define BOOL_INPUT(index) (circa::as_bool(INPUT(index)))
#define STRING_INPUT(index) (circa::as_string(INPUT(index)).c_str())
#define INT_INPUT(index) (circa::as_int(INPUT(index)))
#define CONSUME_INPUT(index, dest) (consume_input(_cxt, INPUT_TERM(index), (dest)));
#define STATE_INPUT (get_state_input(_cxt, CALLER))
#define OUTPUT (_vals[_ninputs])
#define OUTPUT_NTH(index) (_vals[_ninputs+(index)])
#define EXTRA_OUTPUT(index) (_vals[_ninputs+1+(index)])
#define FUNCTION (CALLER->function)
#define RAISE_ERROR(msg) (raise_error(_cxt, (msg)))
