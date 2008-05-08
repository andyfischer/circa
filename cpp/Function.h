#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

#include "CommonHeaders.h"

#include "Term.h"

namespace function {

struct Function
{
   // Term pointers
   TermList input_types;
   Term* output_type;

   // Attributes
   bool pure_function;
   bool has_state;
   string name;

   // Code
   void (*evaluate)(Term*);

public:
   Function() :
      pure_function(false),
      has_state(false),
      output_type(NULL),
      name("undefined")
   {}
};

void initialize_data(Term* func);
string to_string(Term* func);

// Setters
void set_name(Term* func, string name);
void set_input_type(Term* func, int index, Term* type);
void set_output_type(Term* func, Term* type);
void set_evaluate_func(Term* func, void(*evaluate)(Term*));

// Accessors
Term* output_type(Term* func);
bool pure_function(Term* func);
bool has_state(Term* func);
string name(Term* func);
void (*evaluate_func(Term* func))(Term*);

// Functions

} // namespace function

#endif // FUNCTION_H_INCLUDED
