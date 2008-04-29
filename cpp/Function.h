#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

#include "CommonHeaders.h"

#include "Term.h"

namespace function {

struct Function
{
   TermList _inputTypes;
   Term* _outputType;
   bool _pureFunction;
   bool _hasState;
   string _name;

public:
   Function() :
      _pureFunction(false),
      _hasState(false)
   {
      _name = "undefined";
      _outputType = NULL;
   }
};

void initialize_data(Term* func);
void set_name(Term* func, string name);
void set_input_type(Term* func, int index, Term* type);
void set_output_type(Term* func, Term* type);

} // namespace function

#endif // FUNCTION_H_INCLUDED
