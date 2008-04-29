
#include "CodeUnit.h"

namespace codeunit {

static Term* _new_term(CodeUnit* code);

// Public functions:

Term* bootstrap_empty_term(CodeUnit* code)
{
   return _new_term(code);
}

Term* create_term(CodeUnit* code, Term* function)
{
   Term* term = _new_term(code);
   term->_function = function;
   return term;
}

void set_input(CodeUnit* code, Term* term, int index, Term* input)
{
   term->_inputs[index] = input;
}

void bind_name(CodeUnit* code, Term* term, string name)
{
   // code->_namespace[name] = term;
}

Term* _new_term(CodeUnit* code)
{
   Term new_term;
   code->_allTerms.push_back(new_term);
   return &_allTerms[_allTerms.size() - 1];
}

} // namespace codeunit
