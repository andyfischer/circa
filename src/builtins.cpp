// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "filesystem.h"
#include "importing.h"
#include "importing_macros.h"
#include "parser.h"
#include "refactoring.h"
#include "term.h"
#include "type.h"

#include "types/ref.h"

namespace circa {

extern "C" {

Branch* KERNEL = NULL;

// STDLIB_CA_TEXT is defined in generated/stdlib_script_text.cpp
extern const char* STDLIB_CA_TEXT;

Term* ADD_FUNC = NULL;
Term* ADDITIONAL_OUTPUT_FUNC = NULL;
Term* ALIAS_FUNC = NULL;
Term* ASSIGN_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* BREAK_FUNC = NULL;
Term* CAST_FUNC = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONTINUE_FUNC = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DISCARD_FUNC = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* ERRORED_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FINISH_MINOR_BRANCH_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* FOR_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_INDEX_FROM_BRANCH_FUNC = NULL;
Term* GET_STATE_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* INSTANCE_FUNC = NULL;
Term* JOIN_FUNC = NULL;
Term* LAMBDA_FUNC = NULL;
Term* LENGTH_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* LIST_APPEND_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_FUNC = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* PRESERVE_STATE_RESULT_FUNC = NULL;
Term* RANGE_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* RETURN_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SUBROUTINE_OUTPUT_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TYPE_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* UNSAFE_ASSIGN_FUNC = NULL;
Term* VALUE_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* NULL_T_TERM = NULL;
Term* RECT_I_TYPE_TERM = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* FUNCTION_ATTRS_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* SYMBOL_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE;

} // extern "C"

// Builtin type objects:
Type ANY_T;
Type BOOL_T;
Type BRANCH_T;
Type DICT_T;
Type ERROR_T;
Type EVAL_CONTEXT_T;
Type FLOAT_T;
Type FUNCTION_T;
Type FUNCTION_ATTRS_T;
Type HANDLE_T;
Type INT_T;
Type LIST_T;
Type NULL_T;
Type OPAQUE_POINTER_T;
Type REF_T;
Type STRING_T;
Type SYMBOL_T;
Type TYPE_T;
Type VOID_T;

// Builtin symbols:
TaggedValue OUT_SYMBOL;
TaggedValue REPEAT_SYMBOL;
TaggedValue UNKNOWN_SYMBOL;

Type* FILE_SIGNATURE_T;

List g_commandLineArguments;

// Standard library functions

CA_FUNCTION(file__modified_time)
{
    set_int(OUTPUT, get_modified_time(STRING_INPUT(0)));
}

CA_FUNCTION(input_func)
{
    int index = INT_INPUT(0);
    TaggedValue* input = CONTEXT->inputStack.getLast()->getIndex(index);
    if (input == NULL)
        return ERROR_OCCURRED("invalid input index");
    copy(input, OUTPUT);
}

CA_FUNCTION(refactor__rename)
{
    rename(as_ref(INPUT(0)), as_string(INPUT(1)));
}

CA_FUNCTION(refactor__change_function)
{
    change_function(as_ref(INPUT_TERM(0)), INPUT_TERM(1));
}

CA_FUNCTION(reflect__this_branch)
{
    set_branch(OUTPUT, CALLER->owningBranch);
}

CA_FUNCTION(length)
{
    set_int(OUTPUT, num_elements(INPUT(0)));
}

CA_FUNCTION(type_func)
{
    set_type(OUTPUT, declared_type(INPUT_TERM(0)));
}

CA_FUNCTION(typename_func)
{
    set_string(OUTPUT, declared_type(INPUT_TERM(0))->name);
}

CA_FUNCTION(input__func)
{
    int index = INT_INPUT(0);
}

void install_standard_library(Branch& kernel)
{
    // Parse the stdlib script
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install each function
    install_function(kernel["file:modified_time"], file__modified_time);
    install_function(kernel["input"], input_func);
    install_function(kernel["length"], length);
    install_function(kernel["type"], type_func);
    install_function(kernel["typename"], typename_func);
    install_function(kernel["refactor:rename"], refactor__rename);
    install_function(kernel["refactor:change_function"], refactor__change_function);
    install_function(kernel["reflect:this_branch"], reflect__this_branch);

    LENGTH_FUNC = kernel["length"];
    TYPE_FUNC = kernel["type"];
}

} // namespace circa
