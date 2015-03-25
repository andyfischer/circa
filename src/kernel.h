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
    Term* annotate;
    Term* annotate_block;
    Term* blank_list;
    Term* break_func;
    Term* case_func;
    Term* case_condition_bool;
    Term* cast;
    Term* closure_block;
    Term* comment;
    Term* cond;
    Term* continue_func;
    Term* copy;
    Term* cull_state;
    Term* declare_field;
    Term* declared_state;
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
    Term* func_apply_method;
    Term* func_call;
    Term* func_call_method;
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
    Term* require_check;
    Term* require_local;
    Term* include_func;
    Term* input;
    Term* inputs_fit_function;
    Term* lambda;
    Term* length;
    Term* less_than;
    Term* less_than_eq;
    Term* list_append;
    Term* loop_condition_bool;
    Term* loop_iterator;
    Term* load_script;
    Term* make;
    Term* make_list;
    Term* make_map;
    Term* map_get;
    Term* method_lookup;
    Term* module_get;
    Term* mult;
    Term* native_patch;
    Term* neg;
    Term* not_equals;
    Term* not_func;
    Term* or_func;
    Term* output;
    Term* overload_error_no_match;
    Term* package;
    Term* range;
    Term* remainder;
    Term* return_func;
    Term* section_block;
    Term* selector;
    Term* to_seq;
    Term* set_index;
    Term* set_field;
    Term* set_with_selector;
    Term* static_error;
    Term* sub;
    Term* sub_i;
    Term* sub_f;
    Term* switch_func;
    Term* syntax_error;
    Term* type;
    Term* type_make;
    Term* unbound_input;
    Term* unknown_function;
    Term* unknown_function_prelude;
    Term* unknown_identifier;
    Term* upvalue;
    Term* vm_save_declared_state;
    Term* vm_close_stateful_minor_frame;
    Term* value;
    Term* while_loop;
};

struct BuiltinTypes {
    Type* any;
    Type* blob;
    Type* block;
    Type* bool_type;
    Type* color;
    Type* error;
    Type* file_signature;
    Type* float_type;
    Type* func;
    Type* int_type;
    Type* list;
    Type* map;
    Type* module_ref;
    Type* native_ptr;
    Type* null;
    Type* opaque_pointer;
    Type* vec2;
    Type* selector;
    Type* stack;
    Type* string;
    Type* symbol;
    Type* term;
    Type* type;
    Type* vm;
    Type* void_type;
};

extern Value* str_evaluationEmpty;
extern Value* str_hasEffects;
extern Value* str_origin;

extern BuiltinFuncs FUNCS;
extern BuiltinTypes TYPES;

World* global_world();
Block* global_builtins_block();

void empty_evaluate_function(Term* caller);

// Interact with special debugging functions test_spy() and test_oracle()
void test_spy_clear();
Value* test_spy_get_results();
void test_oracle_clear();
Value* test_oracle_append();
void test_oracle_send(Value* value);
void test_oracle_send(int i);

void install_standard_library(Block* kernel);
void on_new_function_parsed(Term* func, Value* functionName);

// find_builtin_file is defined in generated/stdlib_script_text.cpp
const char* find_builtin_file(const char* name);

} // namespace circa
