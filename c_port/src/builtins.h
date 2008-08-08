#ifndef CIRCA__BOOTSTRAP__INCLUDED
#define CIRCA__BOOTSTRAP__INCLUDED

#include "common_headers.h"

namespace circa {

extern Branch* KERNEL;

extern Term* CONST_GENERATOR;
extern Term* INT_TYPE;
extern Term* FLOAT_TYPE;
extern Term* BOOL_TYPE;
extern Term* STRING_TYPE;
extern Term* TYPE_TYPE;
extern Term* FUNCTION_TYPE;
extern Term* CODEUNIT_TYPE;
extern Term* SUBROUTINE_TYPE;
extern Term* STRUCT_DEFINITION_TYPE;
extern Term* BRANCH_TYPE;
extern Term* ANY_TYPE;
extern Term* VOID_TYPE;
extern Term* REFERENCE_TYPE;
extern Term* LIST_TYPE;
extern Term* CONSTANT_INT;
extern Term* CONSTANT_STRING;
extern Term* CONSTANT_0;
extern Term* CONSTANT_1;
extern Term* CONSTANT_2;

Term* get_global(std::string name);
void empty_execute_function(Term* caller);
void initialize();

Term*& as_reference(Term* term);

} // namespace circa

#endif
