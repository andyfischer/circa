// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

struct BuiltinFuncs {
    Term* add;
    Term* add_i;
    Term* add_f;
    Term* block_dynamic_call;
    Term* block_unevaluated;
    Term* break_func;
    Term* case_func;
    Term* cast;
    Term* closure_block;
    Term* closure_call;
    Term* comment;
    Term* cond;
    Term* continue_func;
    Term* copy;
    Term* declare_field;
    Term* declared_state;
    Term* default_case;
    Term* discard;
    Term* div;
    Term* dll_patch;
    Term* dynamic_call;
    Term* dynamic_method;
    Term* error;
    Term* errored;
    Term* exit_point;
    Term* extra_output;
    Term* feedback;
    Term* for_func;
    Term* get_field;
    Term* get_index;
    Term* get_with_selector;
    Term* has_effects;
    Term* if_block;
    Term* require;
    Term* include_func;
    Term* input;
    Term* input_explicit;
    Term* inputs_fit_function;
    Term* lambda;
    Term* length;
    Term* list;
    Term* list_append;
    Term* native_patch;
    Term* load_script;
    Term* loop_index;
    Term* loop_iterator;
    Term* loop_output_index;
    Term* module;
    Term* mult;
    Term* namespace_func;
    Term* neg;
    Term* not_func;
    Term* output;
    Term* output_explicit;
    Term* overload_error_no_match;
    Term* pack_state;
    Term* pack_state_to_list;
    Term* pack_state_list_n;
    Term* package;
    Term* range;
    Term* return_func;
    Term* section_block;
    Term* selector;
    Term* set_index;
    Term* set_field;
    Term* set_with_selector;
    Term* static_error;
    Term* sub;
    Term* switch_func;
    Term* type;
    Term* unbound_input;
    Term* unbounded_loop;
    Term* unbounded_loop_finish;
    Term* unknown_function;
    Term* unknown_identifier;
    Term* unpack_state;
    Term* unpack_state_from_list;
    Term* unpack_state_list_n;
    Term* unrecognized_expression;
    Term* value;
};

struct BuiltinTypes {
    Type* actor;
    Type* any;
    Type* block;
    Type* bool_type;
    Type* closure;
    Type* color;
    Type* dict;
    Type* error;
    Type* eval_context;
    Type* file_signature;
    Type* frame;
    Type* float_type;
    Type* function;
    Type* int_type;
    Type* list;
    Type* map;
    Type* null;
    Type* opaque_pointer;
    Type* point;
    Type* term;
    Type* selector;
    Type* string;
    Type* symbol;
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

void empty_evaluate_function(Term* caller);

namespace assign_function {
    void update_assign_contents(Term* term);
}

namespace copy_function {
    CA_FUNCTION(evaluate);
}

namespace file_changed_function {
    bool check(Stack*, Term* caller, caValue* fileSignature,
            std::string const& filename);
}

namespace for_function {
    std::string get_heading_source(Term* term);
}

namespace namespace_function {
    void early_setup(Block* kernel);
}

namespace neg_function {
    void formatSource(caValue* source, Term* term);
}

namespace return_function {
    void setup(Block* kernel);
}

namespace value_function {
    CA_FUNCTION(evaluate);
}

// Interact with special debugging functions test_spy() and test_oracle()
void test_spy_clear();
caValue* test_spy_get_results();
void test_oracle_clear();
void test_oracle_send(caValue* value);
void test_oracle_send(int i);

void install_standard_library(Block* kernel);

} // namespace circa
