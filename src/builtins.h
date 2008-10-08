// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BOOTSTRAP_INCLUDED
#define CIRCA_BOOTSTRAP_INCLUDED

#include "common_headers.h"

namespace circa {

extern Branch* KERNEL;

extern Term* VAR_FUNCTION_GENERATOR;
extern Term* INT_TYPE;
extern Term* FLOAT_TYPE;
extern Term* BOOL_TYPE;
extern Term* STRING_TYPE;
extern Term* TYPE_TYPE;
extern Term* FUNCTION_TYPE;
extern Term* CODEUNIT_TYPE;
extern Term* BRANCH_TYPE;
extern Term* ANY_TYPE;
extern Term* VOID_TYPE;
extern Term* REFERENCE_TYPE;
extern Term* LIST_TYPE;
extern Term* MAP_TYPE;
extern Term* VAR_INT;
extern Term* VAR_FLOAT;
extern Term* VAR_STRING;
extern Term* VAR_BOOL;
extern Term* CONSTANT_0;
extern Term* CONSTANT_1;
extern Term* CONSTANT_2;
extern Term* CONSTANT_TRUE;
extern Term* CONSTANT_FALSE;
extern Term* UNKNOWN_FUNCTION;

Term* get_global(std::string name);
void empty_evaluate_function(Term* caller);
void initialize();
void shutdown();

int& as_int(Term* t);
float& as_float(Term* t);
bool& as_bool(Term* t);
std::string& as_string(Term* t);
Term*& as_ref(Term* term);

} // namespace circa

#endif
