#ifndef CIRCA__BOOTSTRAP__INCLUDED
#define CIRCA__BOOTSTRAP__INCLUDED

#include "common_headers.h"

struct Branch;
struct Term;

extern Branch* KERNEL;

extern Term* BUILTIN_INT_TYPE;
extern Term* BUILTIN_FLOAT_TYPE;
extern Term* BUILTIN_BOOL_TYPE;
extern Term* BUILTIN_STRING_TYPE;
extern Term* BUILTIN_TYPE_TYPE;
extern Term* BUILTIN_FUNCTION_TYPE;
extern Term* BUILTIN_CODEUNIT_TYPE;
extern Term* BUILTIN_SUBROUTINE_TYPE;
extern Term* BUILTIN_STRUCT_DEFINITION_TYPE;
extern Term* BUILTIN_BRANCH_TYPE;
extern Term* BUILTIN_ANY_TYPE;
extern Term* BUILTIN_VOID_TYPE;
extern Term* BUILTIN_REFERENCE_TYPE;
extern Term* BUILTIN_LIST_TYPE;

Term* get_global(std::string name);
void empty_execute_function(Term* caller);
void initialize();

#endif
