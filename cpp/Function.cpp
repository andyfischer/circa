
#include "Function.h"

namespace function {

#define FUNC_DATA(t) (reinterpret_cast<Function*>((t)->data))

void initialize_data(Term* func)
{
   func->data = new Function();
}

string to_string(Term* func)
{
    std::stringstream sstream;
    sstream << "<Function " << FUNC_DATA(func)->name << ">";
    return sstream.str();
}

void set_name(Term* func, string name)
{
   FUNC_DATA(func)->name = name;
   func->debug_name = name;
}

void set_input_type(Term* func, int index, Term* type)
{
   assert(func != NULL);
   FUNC_DATA(func)->input_types.set(index, type);
}

void set_output_type(Term* func, Term* type)
{
   FUNC_DATA(func)->output_type = type;
}

void set_evaluate_func(Term* func, void(*evaluate)(Term*))
{
   FUNC_DATA(func)->evaluate = evaluate;
}

string name(Term* func)
{
   return FUNC_DATA(func)->name;
}

Term* output_type(Term* func)
{
   return FUNC_DATA(func)->output_type;
}

bool pure_function(Term* func)
{
   return FUNC_DATA(func)->pure_function;
}

bool has_state(Term* func)
{
   return FUNC_DATA(func)->has_state;
}

void (*evaluate_func(Term* func))(Term*)
{
   return FUNC_DATA(func)->evaluate;
}

}
