
#include "CommonHeaders.h"

#include "Builtins.h"
#include "CodeUnit.h"
#include "Function.h"
#include "Type.h"

namespace builtins {

static codeunit::CodeUnit* KERNEL = NULL;

void bootstrap()
{
   KERNEL = new codeunit::CodeUnit;

   // Create constant-generator function. This function takes a Type as
   // input, and gives a Function as output.
   Term* constFuncGenerator = codeunit::bootstrap_empty_term(KERNEL);
   function::initialize_data(constFuncGenerator);
   function::set_name(constFuncGenerator, "constant-generator");
   
   // Create constant-Type function. This function outputs a Type.
   Term* constType = codeunit::bootstrap_empty_term(KERNEL);
   constType->function = constFuncGenerator;
   function::initialize_data(constType);
   function::set_name(constType, "constant-Type");

   // Create Type type
   Term* typeType = codeunit::bootstrap_empty_term(KERNEL);
   typeType->function = constType;
   codeunit::bind_name(KERNEL, typeType, "Type");
   type::initialize_data_for_types(typeType);
   type::set_initialize_data_func(typeType, type::initialize_data_for_types);

   // Implant the Type type
   codeunit::set_input(KERNEL, constType, 0, typeType);
   function::set_input_type(constFuncGenerator, 0, typeType);
   function::set_output_type(constType, typeType);

   // Create constant-Function function, which outputs Functions.
   Term* constFuncFunc = codeunit::bootstrap_empty_term(KERNEL);
   constFuncFunc->function = constFuncGenerator;
   function::initialize_data(constFuncFunc);
   function::set_name(constFuncFunc, "constant-Function");

   // Create Function type
   Term* funcType = codeunit::bootstrap_empty_term(KERNEL);
   funcType->function = constFuncFunc;
   type::initialize_data_for_types(funcType);
   codeunit::bind_name(KERNEL, funcType, "Function");

   // Implant constant-Function
   codeunit::set_input(KERNEL, constFuncGenerator, 0, constFuncFunc);
   function::set_output_type(constFuncFunc, funcType);

   // Implant Function type
   codeunit::set_input(KERNEL, constFuncFunc, 0, funcType);
   function::set_output_type(constFuncGenerator, funcType);
}

}
