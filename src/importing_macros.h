// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

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

#define CA_DEFINE_FUNCTION(fname, header) \
    void evaluate_##fname(EvalContext* _circa_ec, Term* _circa_caller); \
    static _circa_StaticFuncDeclaration _static_decl_for_##fname(header, evaluate_##fname); \
    void evaluate_##fname(EvalContext* _circa_ec, Term* _circa_caller)

#define INT_INPUT(index) int_input(_circa_caller, index)
#define INPUT(index) (_circa_caller->input(index))


#define CA_SETUP_FUNCTIONS(branch) {\
    for (size_t i=0; i < _circa_START_FUNCTIONS.size(); i++) \
        import_function(branch, _circa_START_FUNCTIONS[i]->_func,\
                _circa_START_FUNCTIONS[i]->_header);\
    }
