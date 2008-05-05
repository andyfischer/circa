
#include "CommonHeaders.h"

#include "Builtins.h"
#include "CodeUnit.h"
#include "Function.h"
#include "Type.h"

namespace builtins {

static codeunit::CodeUnit* KERNEL = NULL;
codeunit::CodeUnit* BUILTINS = NULL;

Term* CONST_GENERATOR = NULL;
Term* FUNCTION_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* STRING_TYPE = NULL;

void bootstrap_kernel()
{
   KERNEL = new codeunit::CodeUnit;

   // Create constant-generator function. This function takes a Type as
   // input, and gives a Function as output.
   CONST_GENERATOR = codeunit::bootstrap_empty_term(KERNEL);
   function::initialize_data(CONST_GENERATOR);
   CONST_GENERATOR->function = CONST_GENERATOR;
   function::set_name(CONST_GENERATOR, "constant-generator");

   // Create constant-Type function. This function outputs a Type.
   Term* constType = codeunit::bootstrap_empty_term(KERNEL);
   constType->function = CONST_GENERATOR;
   function::initialize_data(constType);
   function::set_name(constType, "constant-Type");

   // Create Type type
   TYPE_TYPE = codeunit::bootstrap_empty_term(KERNEL);
   TYPE_TYPE->function = constType;
   codeunit::bind_name(KERNEL, TYPE_TYPE, "Type");
   type::initialize_data_for_types(TYPE_TYPE);
   type::set_name(TYPE_TYPE, "Type");
   type::set_initialize_data_func(TYPE_TYPE, type::initialize_data_for_types);
   type::set_to_string_func(TYPE_TYPE, type::to_string);

   // Implant the Type type
   codeunit::set_input(KERNEL, constType, 0, TYPE_TYPE);
   function::set_input_type(CONST_GENERATOR, 0, TYPE_TYPE);
   function::set_output_type(constType, TYPE_TYPE);

   // Create constant-Function function, which outputs Functions.
   Term* constFuncFunc = codeunit::bootstrap_empty_term(KERNEL);
   constFuncFunc->function = CONST_GENERATOR;
   function::initialize_data(constFuncFunc);
   function::set_name(constFuncFunc, "constant-Function");

   // Create Function type
   FUNCTION_TYPE = codeunit::bootstrap_empty_term(KERNEL);
   FUNCTION_TYPE->function = constFuncFunc;
   type::initialize_data_for_types(FUNCTION_TYPE);
   type::set_name(FUNCTION_TYPE, "Function");
   type::set_initialize_data_func(FUNCTION_TYPE, function::initialize_data);
   type::set_to_string_func(FUNCTION_TYPE, function::to_string);
   codeunit::bind_name(KERNEL, FUNCTION_TYPE, "Function");

   // Implant constant-Function
   codeunit::set_input(KERNEL, CONST_GENERATOR, 0, constFuncFunc);
   function::set_output_type(constFuncFunc, FUNCTION_TYPE);

   // Implant Function type
   codeunit::set_input(KERNEL, constFuncFunc, 0, FUNCTION_TYPE);
   function::set_output_type(CONST_GENERATOR, FUNCTION_TYPE);
}

void bootstrap_builtins()
{
    cout << CONST_GENERATOR->to_string() << endl;
    cout << TYPE_TYPE->to_string() << endl;

    BUILTINS = new codeunit::CodeUnit;

    // Create primitives
    // Create int type
    INT_TYPE = codeunit::create_term(BUILTINS, CONST_GENERATOR, TermList(TYPE_TYPE));
    type::set_initialize_data_func(INT_TYPE, int_type::initialize_data);
    type::set_to_string_func(INT_TYPE, int_type::to_string);

    // Create string type
    STRING_TYPE = codeunit::create_term(BUILTINS, CONST_GENERATOR, TermList(TYPE_TYPE));
    type::set_initialize_data_func(STRING_TYPE, string_type::initialize_data);
    type::set_to_string_func(STRING_TYPE, string_type::to_string);
}

}
