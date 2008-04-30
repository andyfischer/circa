
#include "CodeUnit.h"
#include "Function.h"
#include "Type.h"

namespace codeunit {

static Term* _new_term(CodeUnit* code);

Term* bootstrap_empty_term(CodeUnit* code)
{
   return _new_term(code);
}

Term* create_term(CodeUnit* code, Term* func)
{
   Term* new_term = _new_term(code);
   new_term->function = func;

   assert(func != NULL);
   assert(function::output_type(func) != NULL);

   type::call_initialize_data(function::output_type(func), new_term);

   return new_term;
}

void set_input(CodeUnit* code, Term* term, int index, Term* input)
{
   term->inputs.set(index, input);
}

void bind_name(CodeUnit* code, Term* term, string name)
{
   code->term_namespace[name] = term;
}

Term* _new_term(CodeUnit* code)
{
   Term* term = new Term;
   code->all_terms.push_back(term);
   return term;
}

} // namespace codeunit
