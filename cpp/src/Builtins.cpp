
#include "CommonHeaders.h"

#include "Builtins.h"
#include "CodeUnit.h"
#include "Errors.h"
#include "Function.h"
#include "Type.h"
#include "TermIterator.h"

namespace builtins {

static codeunit::CodeUnit* KERNEL = NULL;
static void sanity_check();

codeunit::CodeUnit* BUILTINS = NULL;

Term* FUNCTION_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* STRING_TYPE = NULL;

Term* CONST_GENERATOR = NULL;
Term* CONST_FUNC_FUNC = NULL;
Term* CONST_TYPE_FUNC = NULL;

/*
  constant-generator = 
    Type -> ( constant-generator ) -> Function

  constant-Function =
    (empty) -> ( constant-Function ) -> Function


       input            function             result

  FUNCTION_TYPE -> ( CONST_GENERATOR ) -> CONST_FUNC_FUNC
    TYPE_TYPE   -> ( CONST_GENERATOR ) -> CONST_TYPE_FUNC

      (empty)   -> ( CONST_FUNC_FUNC ) -> CONST_GENERATOR

      (empty)   -> ( CONST_TYPE_FUNC ) -> FUNCTION_TYPE
      (empty)   -> ( CONST_TYPE_FUNC ) ->   TYPE_TYPE

 */

void const_generator_evaluate(Term* term)
{
    Term* input_type = term->input(0);
    function::set_output_type(term, input_type);
}

void bootstrap_kernel()
{
   KERNEL = new codeunit::CodeUnit;

   // Create constant-generator function. This function takes a Type as
   // input, and gives a Function as output.
   CONST_GENERATOR = codeunit::bootstrap_empty_term(KERNEL);
   function::initialize_data(CONST_GENERATOR);
   CONST_GENERATOR->function = CONST_GENERATOR;
   function::set_name(CONST_GENERATOR, "constant-generator");

   // Create constant-Type function
   Term* CONST_TYPE_FUNC = codeunit::bootstrap_empty_term(KERNEL);
   CONST_TYPE_FUNC->function = CONST_GENERATOR;
   function::initialize_data(CONST_TYPE_FUNC);
   function::set_name(CONST_TYPE_FUNC, "constant-Type");

   // Create Type type
   TYPE_TYPE = codeunit::bootstrap_empty_term(KERNEL);
   TYPE_TYPE->function = CONST_TYPE_FUNC;
   codeunit::bind_name(KERNEL, TYPE_TYPE, "Type");
   type::initialize_data_for_types(TYPE_TYPE);
   type::set_name(TYPE_TYPE, "Type");
   type::set_initialize_data_func(TYPE_TYPE, type::initialize_data_for_types);
   type::set_to_string_func(TYPE_TYPE, type::to_string);

   // Implant the Type type
   codeunit::set_input(KERNEL, CONST_TYPE_FUNC, 0, TYPE_TYPE);
   function::set_input_type(CONST_GENERATOR, 0, TYPE_TYPE);

   // Create constant-Function function.
   Term* CONST_FUNC_FUNC = codeunit::bootstrap_empty_term(KERNEL);
   CONST_FUNC_FUNC->function = CONST_GENERATOR;
   function::initialize_data(CONST_FUNC_FUNC);
   function::set_name(CONST_FUNC_FUNC, "constant-Function");

   // Create Function type
   FUNCTION_TYPE = codeunit::bootstrap_empty_term(KERNEL);
   FUNCTION_TYPE->function = CONST_FUNC_FUNC;
   type::initialize_data_for_types(FUNCTION_TYPE);
   type::set_name(FUNCTION_TYPE, "Function");
   type::set_initialize_data_func(FUNCTION_TYPE, function::initialize_data);
   type::set_to_string_func(FUNCTION_TYPE, function::to_string);
   codeunit::bind_name(KERNEL, FUNCTION_TYPE, "Function");

   // Implant constant-Function
   codeunit::set_input(KERNEL, CONST_GENERATOR, 0, CONST_FUNC_FUNC);

   // Implant Function type
   codeunit::set_input(KERNEL, CONST_FUNC_FUNC, 0, FUNCTION_TYPE);
   function::set_output_type(CONST_GENERATOR, FUNCTION_TYPE);
   function::set_output_type(CONST_FUNC_FUNC, FUNCTION_TYPE);
   function::set_output_type(CONST_TYPE_FUNC, FUNCTION_TYPE);

   // Define constant-generator
   function::set_evaluate_func(CONST_GENERATOR, const_generator_evaluate);

   sanity_check();
}

void bootstrap_builtins()
{
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

void sanity_check()
{
    if (KERNEL == NULL) {
        cout << "sanity_check: KERNEL is NULL\n";
    } else {
        for (int index=0; index < KERNEL->all_terms.size(); index++)
        {
            Term* term = KERNEL->all_terms[index];

            if (term->function == NULL) {
                INTERNAL_ERROR(string("Function is NULL: ") + term->debug_identifier());
            }

            if (term->data == NULL) {
                INTERNAL_ERROR(string("Data is NULL: ") + term->debug_identifier());
            }
        }
    }
}

}
