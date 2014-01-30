// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

struct BuiltinFuncs {
    Term* add;
    Term* add_i;
    Term* add_f;
    Term* and_func;
    Term* block_unevaluated;
    Term* break_func;
    Term* case_func;
    Term* case_condition_bool;
    Term* cast;
    Term* closure_block;
    Term* comment;
    Term* cond;
    Term* continue_func;
    Term* copy;
    Term* declare_field;
    Term* declared_state;
    Term* default_case;
    Term* discard;
    Term* div;
    Term* div_f;
    Term* div_i;
    Term* dynamic_method;
    Term* dynamic_term_eval;
    Term* error;
    Term* equals;
    Term* extra_output;
    Term* eval_on_demand;
    Term* feedback;
    Term* for_func;
    Term* func_apply;
    Term* func_call;
    Term* func_call_implicit;
    Term* function_decl;
    Term* get_field;
    Term* get_index;
    Term* get_with_selector;
    Term* get_with_symbol;
    Term* greater_than;
    Term* greater_than_eq;
    Term* has_effects;
    Term* if_block;
    Term* require;
    Term* include_func;
    Term* input;
    Term* input_explicit;
    Term* inputs_fit_function;
    Term* lambda;
    Term* length;
    Term* less_than;
    Term* less_than_eq;
    Term* list;
    Term* list_append;
    Term* loop_condition_bool;
    Term* load_script;
    Term* loop_index;
    Term* loop_iterator;
    Term* loop_output_index;
    Term* looped_input;
    Term* memoize;
    Term* minor_return_if_empty;
    Term* module;
    Term* mult;
    Term* native_patch;
    Term* neg;
    Term* nonlocal;
    Term* not_equals;
    Term* not_func;
    Term* or_func;
    Term* output;
    Term* output_explicit;
    Term* overload_error_no_match;
    Term* package;
    Term* range;
    Term* remainder;
    Term* return_func;
    Term* section_block;
    Term* selector;
    Term* set_index;
    Term* set_field;
    Term* set_with_selector;
    Term* static_error;
    Term* sub;
    Term* sub_i;
    Term* sub_f;
    Term* switch_func;
    Term* type;
    Term* unbound_input;
    Term* unknown_function;
    Term* unknown_identifier;
    Term* unrecognized_expression;
    Term* value;
    Term* while_loop;
};

struct BuiltinTypes {
    Type* any;
    Type* block;
    Type* blob;
    Type* bool_type;
    Type* color;
    Type* error;
    Type* file_signature;
    Type* frame;
    Type* float_type;
    Type* func;
    Type* int_type;
    Type* list;
    Type* map;
    Type* module_ref;
    Type* module_frame;
    Type* mutable_type;
    Type* null;
    Type* opaque_pointer;
    Type* point;
    Type* retained_frame;
    Type* selector;
    Type* stack;
    Type* string;
    Type* symbol;
    Type* term;
    Type* type;
    Type* void_type;
};

extern caValue* str_evaluationEmpty;
extern caValue* str_hasEffects;
extern caValue* str_origin;

extern BuiltinFuncs FUNCS;
extern BuiltinTypes TYPES;

World* global_world();
Block* global_root_block();
Block* global_builtins_block();

void empty_evaluate_function(Term* caller);

namespace assign_function {
    void update_assign_contents(Term* term);
}

namespace copy_function {
    void evaluate(caStack* stack);
}

namespace file_changed_function {
    bool check(Stack*, Term* caller, caValue* fileSignature,
            std::string const& filename);
}

namespace for_function {
    std::string get_heading_source(Term* term);
}

namespace neg_function {
    void formatSource(caValue* source, Term* term);
}

namespace return_function {
    void setup(Block* kernel);
}

// Interact with special debugging functions test_spy() and test_oracle()
void test_spy_clear();
caValue* test_spy_get_results();
void test_oracle_clear();
caValue* test_oracle_append();
void test_oracle_send(caValue* value);
void test_oracle_send(int i);

void install_standard_library(Block* kernel);
void on_new_function_parsed(Term* func, caValue* functionName);

// find_builtin_module is defined in generated/stdlib_script_text.cpp
const char* find_builtin_module(const char* name);

} // namespace circa
