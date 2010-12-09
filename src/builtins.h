// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

extern "C" {

extern Branch* KERNEL;

extern Term* ALIAS_FUNC;
extern Term* ADD_FUNC;
extern Term* ASSIGN_FUNC;
extern Term* APPLY_FEEDBACK;
extern Term* AVERAGE_FUNC;
extern Term* BRANCH_FUNC;
extern Term* COMMENT_FUNC;
extern Term* COPY_FUNC;
extern Term* CAST_FUNC;
extern Term* DESIRED_VALUE_FEEDBACK;
extern Term* DISCARD_FUNC;
extern Term* DIV_FUNC;
extern Term* DO_ONCE_FUNC;
extern Term* FEEDBACK_FUNC;
extern Term* FREEZE_FUNC;
extern Term* FOR_FUNC;
extern Term* GET_INDEX_FUNC;
extern Term* GET_INDEX_FROM_BRANCH_FUNC;
extern Term* GET_NAMESPACE_FIELD;
extern Term* GET_FIELD_FUNC;
extern Term* IF_FUNC;
extern Term* IF_BLOCK_FUNC;
extern Term* COND_FUNC;
extern Term* INCLUDE_FUNC;
extern Term* INPUT_PLACEHOLDER_FUNC;
extern Term* JOIN_FUNC;
extern Term* LIST_FUNC;
extern Term* MAP_TYPE;
extern Term* MULT_FUNC;
extern Term* NAMESPACE_FUNC;
extern Term* NEG_FUNC;
extern Term* NOT_FUNC;
extern Term* ONE_TIME_ASSIGN_FUNC;
extern Term* OVERLOADED_FUNCTION_FUNC;
extern Term* SET_FIELD_FUNC;
extern Term* SET_INDEX_FUNC;
extern Term* STATEFUL_VALUE_FUNC;
extern Term* SUB_FUNC;
extern Term* RANGE_FUNC;
extern Term* REF_FUNC;
extern Term* RETURN_FUNC;
extern Term* UNKNOWN_FUNCTION;
extern Term* UNKNOWN_FIELD_FUNC;
extern Term* UNKNOWN_IDENTIFIER_FUNC;
extern Term* UNKNOWN_TYPE_FUNC;
extern Term* UNRECOGNIZED_EXPRESSION_FUNC;
extern Term* UNSAFE_ASSIGN_FUNC;
extern Term* VALUE_FUNC;

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
extern Term* FUNCTION_ATTRS_TYPE;
extern Term* LIST_TYPE;
extern Term* TYPE_TYPE;
extern Term* VOID_TYPE;

} // extern "C"

extern TypeRef BOOL_T;
extern TypeRef DICT_T;
extern TypeRef FILE_SIGNATURE_T;
extern TypeRef FLOAT_T;
extern TypeRef INT_T;
extern TypeRef STRING_T;
extern TypeRef REF_T;
extern TypeRef TYPE_T;
extern TypeRef VOID_T;

extern Type NULL_TYPE;
extern Type LIST_T;

extern bool FINISHED_BOOTSTRAP;

Branch& kernel();

void empty_evaluate_function(Term* caller);

namespace assign_function {
    void update_assign_contents(Term* term);
}

namespace copy_function {
    CA_FUNCTION(evaluate);
}

namespace include_function {
    void preload_script(Term* term);
}

namespace file_changed_function {
    bool check(EvalContext*, Term* caller, TaggedValue* fileSignature,
            std::string const& filename);
}

namespace for_function {
    std::string get_heading_source(Term* term);
}

namespace namespace_function {
    void early_setup(Branch& kernel);
}

namespace overloaded_function {

    bool is_overloaded_function(Term* func);
    int num_overloads(Term* func);
    Term* get_overload(Term* func, int index);
    Term* find_overload(Term* func, const char* name);
    Term* create_overloaded_function(Branch& branch, std::string const& name,
        RefList const& overloads);
    void append_overload(Term* overloadedFunction, Term* overload);
    Term* statically_specialize_function(Term* func, RefList const& inputs);
    void post_compile_setup_overloaded_function(Term* term);
}

namespace value_function {
    CA_FUNCTION(evaluate);
}

} // namespace circa

export_func void circa_initialize();
export_func void circa_shutdown();
