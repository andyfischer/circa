// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

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


#define INPUT(index) (get_input(_caller, (index)))
#define FLOAT_INPUT(index) (circa::to_float(INPUT(index)))
#define BOOL_INPUT(index) (circa::as_bool(INPUT(index)))
#define STRING_INPUT(index) (circa::as_string(INPUT(index)).c_str())
#define INT_INPUT(index) (circa::as_int(INPUT(index)))
#define NUM_INPUTS (_caller->numInputs())
#define STATE_INPUT (get_state_input(_context, _caller))
#define CALLER (_caller)
#define INPUT_TERM(index) (CALLER->input(index))
#define OUTPUT (get_output(_caller, 0))
#define EXTRA_OUTPUT(index) (get_extra_output(_caller, (index)))
#define CONTEXT (_context)
#define FUNCTION (_caller->function)
#define ERROR_OCCURRED(msg) (error_occurred(_context, _caller, (msg)))
