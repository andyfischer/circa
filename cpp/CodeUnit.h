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
Term* create_term(CodeUnit* code, Term* function);
void set_input(CodeUnit* code, Term* term, int index, Term* input);
void bind_name(CodeUnit* code, Term* term, string name);

}

#endif
