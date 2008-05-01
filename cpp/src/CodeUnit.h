#ifndef CODE_UNIT_H_INCLUDED
#define CODE_UNIT_H_INCLUDED

#include "CommonHeaders.h"

#include "Branch.h"
#include "Term.h"
#include "TermNamespace.h"

namespace codeunit {

struct CodeUnit
{
   vector<Term*> all_terms;
   Branch main_branch;
   TermNamespace term_namespace;
};

Term* bootstrap_empty_term(CodeUnit* code);
Term* get_term(CodeUnit* code, Term* func, const TermList& inputs);
Term* create_term(CodeUnit* code, Term* function);
Term* create_term(CodeUnit* code, Term* func, const TermList& inputs);
Term* create_constant(CodeUnit* code, Term* type);
void set_input(CodeUnit* code, Term* term, int index, Term* input);
void set_inputs(CodeUnit* code, Term* term, const TermList& inputs);
void bind_name(CodeUnit* code, Term* term, string name);

}

#endif
