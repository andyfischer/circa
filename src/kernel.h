// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

extern Branch* KERNEL;

extern Term* ANY_TYPE;
extern Term* BOOL_TYPE;
extern Term* DICT_TYPE;
extern Term* FLOAT_TYPE;
extern Term* INT_TYPE;
extern Term* REF_TYPE;
extern Term* STRING_TYPE;
extern Term* COLOR_TYPE;
extern Term* FEEDBACK_TYPE;
extern Term* FUNCTION_TYPE;
extern Term* NAME_TYPE;
extern Term* TYPE_TYPE;
extern Term* VOID_TYPE;
extern Term* OPAQUE_POINTER_TYPE;

struct BuiltinFuncs {
    Term* add;
    Term* add_i;
    Term* add_f;
    Term* branch;
    Term* branch_dynamic_call;
    Term* branch_unevaluated;
    Term* break_func;
    Term* case_func;
    Term* cast;
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
    Term* freeze;
    Term* for_func;
    Term* get_field;
    Term* get_index;
    Term* get_with_selector;
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
    Term* selector;
    Term* set_index;
    Term* set_field;
    Term* set_with_selector;
    Term* static_error;
    Term* sub;
    Term* switch_func;
    Term* type;
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

extern BuiltinFuncs FUNCS;

extern Type ANY_T;
extern Type BOOL_T;
extern Type BRANCH_T;
extern Type DICT_T;
extern Type ERROR_T;
extern Type EVAL_CONTEXT_T;
extern Type FLOAT_T;
extern Type FUNCTION_T;
extern Type FUNCTION_ATTRS_T;
extern Type HANDLE_T;
extern Type INT_T;
extern Type LIST_T;
extern Type NULL_T;
extern Type OPAQUE_POINTER_T;
extern Type POINT_T;
extern Type REF_T;
extern Type STRING_T;
extern Type NAME_T;
extern Type TYPE_T;
extern Type VOID_T;

struct BuiltinTypes {
    Type* actor;
    Type* any;
    Type* branch;
    Type* color;
    Type* dict;
    Type* error;
    Type* file_signature;
    Type* frame;
    Type* float_type;
    Type* list;
    Type* map;
    Type* point;
    Type* selector;
    Type* void_type;
};

extern BuiltinTypes TYPES;

extern Value TrueValue;
extern Value FalseValue;

extern bool STATIC_INITIALIZATION_FINISHED;
extern bool FINISHED_BOOTSTRAP;
extern bool SHUTTING_DOWN;

World* global_world();
Branch* global_root_branch();

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
    void early_setup(Branch* kernel);
}

namespace neg_function {
    void formatSource(caValue* source, Term* term);
}

namespace return_function {
    void setup(Branch* kernel);
}

namespace value_function {
    CA_FUNCTION(evaluate);
}

// Interact with special debugging functions test_spy() and test_oracle()
void test_spy_clear();
List* test_spy_get_results();
void test_oracle_clear();
void test_oracle_send(caValue* value);
void test_oracle_send(int i);

void install_standard_library(Branch* kernel);

} // namespace circa
