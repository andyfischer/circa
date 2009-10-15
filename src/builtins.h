// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILTINS_INCLUDED
#define CIRCA_BUILTINS_INCLUDED

#include "common_headers.h"

namespace circa {

extern Branch* KERNEL;

extern Term* ADD_FUNC;
extern Term* ANNOTATE_TYPE_FUNC;
extern Term* ASSIGN_FUNC;
extern Term* APPLY_FEEDBACK;
extern Term* AVERAGE_FUNC;
extern Term* BRANCH_FUNC;
extern Term* COMMENT_FUNC;
extern Term* COPY_FUNC;
extern Term* DESIRED_VALUE_FEEDBACK;
extern Term* DISCARD_FUNC;
extern Term* DIV_FUNC;
extern Term* DO_ONCE_FUNC;
extern Term* FEEDBACK_FUNC;
extern Term* FOR_FUNC;
extern Term* GET_INDEX_FUNC;
extern Term* GET_FIELD_FUNC;
extern Term* IF_FUNC;
extern Term* IF_BLOCK_FUNC;
extern Term* IF_EXPR_FUNC;
extern Term* INCLUDE_FUNC;
extern Term* INPUT_PLACEHOLDER_FUNC;
extern Term* LIST_FUNC;
extern Term* MAP_TYPE;
extern Term* MULT_FUNC;
extern Term* NEG_FUNC;
extern Term* NOT_FUNC;
extern Term* ONE_TIME_ASSIGN_FUNC;
extern Term* SET_FIELD_FUNC;
extern Term* SET_INDEX_FUNC;
extern Term* STATEFUL_VALUE_FUNC;
extern Term* SUB_FUNC;
extern Term* TO_REF_FUNC;
extern Term* UNKNOWN_FUNCTION;
extern Term* UNKNOWN_FIELD_FUNC;
extern Term* UNKNOWN_IDENTIFIER_FUNC;
extern Term* UNKNOWN_TYPE_FUNC;
extern Term* UNRECOGNIZED_EXPRESSION_FUNC;
extern Term* VALUE_FUNC;

extern Term* BRANCH_TYPE;
extern Term* BRANCH_MIRROR_TYPE;
extern Term* CODE_TYPE;
extern Term* COLOR_TYPE;
extern Term* FEEDBACK_TYPE;
extern Term* FUNCTION_TYPE;
extern Term* LIST_TYPE;
extern Term* NAMESPACE_TYPE;
extern Term* OVERLOADED_FUNCTION_TYPE;
extern Term* TYPE_TYPE;
extern Term* VOID_TYPE;

// Get a named term from the global namespace.
Term* get_global(std::string name);

void empty_evaluate_function(Term* caller);
void initialize();
void shutdown();

// this is implemented in builtin_functions/include.cpp
namespace include_function {
    void load_script(Term* caller);
}

namespace for_function {
    std::string get_heading_source(Term* term);
}

} // namespace circa

#endif
