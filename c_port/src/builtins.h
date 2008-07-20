#ifndef __BOOTSTRAP_INCLUDED__
#define __BOOTSTRAP_INCLUDED__

struct CodeUnit;
struct Term;

extern CodeUnit* KERNEL;

extern Term* BUILTIN_INT_TYPE;
extern Term* BUILTIN_FLOAT_TYPE;
extern Term* BUILTIN_BOOL_TYPE;
extern Term* BUILTIN_STRING_TYPE;
extern Term* BUILTIN_TYPE_TYPE;
extern Term* BUILTIN_FUNCTION_TYPE;
extern Term* BUILTIN_CODEUNIT_TYPE;
extern Term* BUILTIN_SUBROUTINE_TYPE;

void initialize();

#endif
