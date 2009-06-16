// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BUILTINS_INCLUDED
#define CIRCA_BUILTINS_INCLUDED

#include "common_headers.h"

namespace circa {

extern Branch* KERNEL;

extern Term* ASSIGN_FUNC;
extern Term* ADD_FUNC;
extern Term* ANNOTATE_TYPE_FUNC;
extern Term* ANY_TYPE;
extern Term* APPLY_FEEDBACK;
extern Term* AVERAGE_FUNC;
extern Term* BOOL_TYPE;
extern Term* BRANCH_FUNC;
extern Term* BRANCH_TYPE;
extern Term* CODEUNIT_TYPE;
extern Term* COMMENT_FUNC;
extern Term* CONSTANT_TRUE;
extern Term* CONSTANT_FALSE;
extern Term* COPY_FUNC;
extern Term* DESIRED_VALUE_FEEDBACK;
extern Term* DIV_FUNC;
extern Term* DO_ONCE_FUNC;
extern Term* FEEDBACK_FUNC;
extern Term* FEEDBACK_TYPE;
extern Term* FLOAT_TYPE;
extern Term* FOR_FUNC;
extern Term* FUNCTION_TYPE;
extern Term* GET_INDEX_FUNC;
extern Term* GET_FIELD_FUNC;
extern Term* IF_FUNC;
extern Term* IF_EXPR_FUNC;
extern Term* INT_TYPE;
extern Term* INPUT_PLACEHOLDER_FUNC;
extern Term* LIST_TYPE;
extern Term* LIST_FUNC;
extern Term* MAP_TYPE;
extern Term* MULT_FUNC;
extern Term* NEG_FUNC;
extern Term* NOT_FUNC;
extern Term* ONE_TIME_ASSIGN_FUNC;
extern Term* OVERLOADED_FUNCTION_TYPE;
extern Term* REF_TYPE;
extern Term* SET_FIELD_FUNC;
extern Term* SET_INDEX_FUNC;
extern Term* SUB_FUNC;
extern Term* SUBROUTINE_TYPE;
extern Term* STRING_TYPE;
extern Term* TYPE_TYPE;
extern Term* UNKNOWN_FUNCTION;
extern Term* UNKNOWN_IDENTIFIER_FUNC;
extern Term* UNKNOWN_TYPE_FUNC;
extern Term* UNRECOGNIZED_EXPRESSION_FUNC;
extern Term* VALUE_FUNC;
extern Term* VOID_TYPE;
extern Term* VOID_PTR_TYPE;

void empty_evaluate_function(Term* caller);
void initialize();
void shutdown();

namespace set_t {
    void add(Branch& branch, Term* value);
}

namespace list_t {
    void append(Branch& branch, Term* value);
}

namespace dict_t {
    std::string to_string(Branch& branch);
}

} // namespace circa

#endif
