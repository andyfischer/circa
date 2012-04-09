// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

extern Branch* KERNEL;

extern Term* APPLY_FEEDBACK;
extern Term* AVERAGE_FUNC;
extern Term* DESIRED_VALUE_FEEDBACK;
extern Term* DO_ONCE_FUNC;
extern Term* ERRORED_FUNC;
extern Term* EXTRA_OUTPUT_FUNC;
extern Term* FEEDBACK_FUNC;
extern Term* FILE_READ_ERROR_FUNC;
extern Term* FREEZE_FUNC;
extern Term* INSTANCE_FUNC;
extern Term* LIST_APPEND_FUNC;
extern Term* MAP_TYPE;
extern Term* OVERLOADED_FUNCTION_FUNC;
extern Term* REF_FUNC;
extern Term* SWITCH_FUNC;
extern Term* STATEFUL_VALUE_FUNC;
extern Term* STATIC_ERROR_FUNC;
extern Term* UNKNOWN_IDENTIFIER_FUNC;
extern Term* UNKNOWN_TYPE_FUNC;
extern Term* UNRECOGNIZED_EXPRESSION_FUNC;

extern Term* ANY_TYPE;
extern Term* BOOL_TYPE;
extern Term* DICT_TYPE;
extern Term* FLOAT_TYPE;
extern Term* INT_TYPE;
extern Term* NULL_T_TERM;
extern Term* REF_TYPE;
extern Term* STRING_TYPE;
extern Term* COLOR_TYPE;
extern Term* FEEDBACK_TYPE;
extern Term* FUNCTION_TYPE;
extern Term* LIST_TYPE;
extern Term* NAME_TYPE;
extern Term* RECT_I_TYPE_TERM;
extern Term* TYPE_TYPE;
extern Term* VOID_TYPE;
extern Term* OPAQUE_POINTER_TYPE;

struct BuiltinFuncs {
    Term* add;
    Term* add_i;
    Term* add_f;
    Term* assign;
    Term* branch;
    Term* branch_unevaluated;
    Term* break_func;
    Term* case_func;
    Term* cast;
    Term* comment;
    Term* cond;
    Term* continue_func;
    Term* copy;
    Term* declared_state;
    Term* default_case;
    Term* discard;
    Term* div;
    Term* dll_patch;
    Term* error;
    Term* for_func;
    Term* get_field;
    Term* get_index;
    Term* if_block;
    Term* import;
    Term* imported_file;
    Term* include_func;
    Term* input;
    Term* input_explicit;
    Term* inputs_fit_function;
    Term* lambda;
    Term* length;
    Term* list;
    Term* load_script;
    Term* loop_index;
    Term* loop_iterator;
    Term* loop_output;
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
    Term* range;
    Term* return_func;
    Term* set_index;
    Term* set_field;
    Term* sub;
    Term* type;
    Term* unbounded_loop;
    Term* unbounded_loop_finish;
    Term* unknown_function;
    Term* unpack_state;
    Term* unpack_state_from_list;
    Term* unpack_state_list_n;
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
    Type* color;
    Type* file_signature;
    Type* point;
};

extern BuiltinTypes TYPES;

extern Value TrueValue;
extern Value FalseValue;

extern bool STATIC_INITIALIZATION_FINISHED;
extern bool FINISHED_BOOTSTRAP;
extern bool SHUTTING_DOWN;

Branch* kernel();

void empty_evaluate_function(Term* caller);

namespace assign_function {
    void update_assign_contents(Term* term);
}

namespace copy_function {
    CA_FUNCTION(evaluate);
}

namespace internal_debug_function {
    void oracle_clear();
    void oracle_send(caValue* value);
    void oracle_send(int i);
    void spy_clear();
    List* spy_results();
}

namespace file_changed_function {
    bool check(EvalContext*, Term* caller, caValue* fileSignature,
            std::string const& filename);
}

namespace for_function {
    std::string get_heading_source(Term* term);
}

namespace namespace_function {
    void early_setup(Branch* kernel);
}

namespace neg_function {
    void formatSource(StyledSource* source, Term* term);
}

namespace return_function {
    void setup(Branch* kernel);
}

namespace value_function {
    CA_FUNCTION(evaluate);
}

void install_standard_library(Branch* kernel);

} // namespace circa
