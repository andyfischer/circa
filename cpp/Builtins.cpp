
#include "CommonHeaders.h"

#include "CodeUnit.h"
#include "Function.h"

#include "Builtins.h"

codeunit::CodeUnit* CODE = NULL;

void bootstrap()
{
   CODE = new codeunit::CodeUnit;

   // Create constant-generator function
   Term* constFuncGenerator = codeunit::bootstrap_empty_term(CODE);
   function::initialize_data(constFuncGenerator);
   function::set_name(constFuncGenerator, "constant-generator");
   
   // Create constant-Type function
   Term* constType = codeunit::create_term(CODE,constFuncGenerator);
   function::initialize_data(constType);
   function::set_name(constType, "constant-Type");

   // Create Type type
   Term* typeType = codeunit::create_term(CODE, constType);
   codeunit::bind_name(CODE,typeType, "Type");

   // Implant the Type type
   codeunit::set_input(CODE,constType, 0, typeType);
   function::set_input_type(constFuncGenerator, 0, typeType);
   function::set_output_type(constType, typeType);

   // Create constant-Function function
   Term* constFuncFunc = codeunit::create_term(CODE,constFuncGenerator);
   function::set_name(constFuncFunc, "constant-Function");

   // Create Function type
   Term* funcType = codeunit::create_term(CODE,constFuncFunc);
   codeunit::bind_name(CODE, funcType, "Function");

   // Implant Function type
   codeunit::set_input(CODE, constFuncFunc, 0, funcType);
}
