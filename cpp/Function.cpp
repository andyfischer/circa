
#include "Function.h"

namespace function {

#define FUNC(t) (reinterpret_cast<Function*>((t)->_data))

void initialize_data(Term* func)
{
   func->_data = new Function;
}

void set_name(Term* func, string name)
{
   FUNC(func)->_name = name;
}

void set_input_type(Term* func, int index, Term* type)
{
   FUNC(func)->_inputTypes->setAt(index, type);
}

void set_output_type(Term* func, Term* type)
{
   FUNC(func)->_outputType = type;
}

}
