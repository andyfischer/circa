
#include "Function.h"

namespace function {

#define FUNC(t) (reinterpret_cast<Function*>((t)->data))

void initialize_data(Term* func)
{
   func->data = new Function();
}

void set_name(Term* func, string name)
{
   FUNC(func)->name = name;
}

void set_input_type(Term* func, int index, Term* type)
{
   assert(func != NULL);
   FUNC(func)->input_types.set(index, type);
}

void set_output_type(Term* func, Term* type)
{
   FUNC(func)->output_type = type;
}

Term* output_type(Term* func)
{
   return FUNC(func)->output_type;
}

}
