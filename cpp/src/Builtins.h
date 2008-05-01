#ifndef BUILTINS_H_INCLUDED
#define BUILTINS_H_INCLUDED

#include "CodeUnit.h"
#include "IntType.h"
#include "Term.h"
#include "StringType.h"

namespace builtins {

void bootstrap_kernel();
void bootstrap_builtins();

extern codeunit::CodeUnit* BUILTINS;

extern Term* CONST_GENERATOR;
extern Term* CONST_TYPE;
extern Term* INT_TYPE;
extern Term* STRING_TYPE;

}


#endif
