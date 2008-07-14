#ifndef CODE_UNIT_H_INCLUDED
#define CODE_UNIT_H_INCLUDED

#include "CommonHeaders.h"

#include "Branch.h"
#include "Term.h"
#include "TermIterator.h"
#include "TermNamespace.h"

namespace codeunit {

class CodeUnitTermIterator;

struct CodeUnit
{
   vector<Term*> all_terms;
   Branch main_branch;
   TermNamespace term_namespace;
};

Term* bootstrap_empty_term(CodeUnit* code);
Term* get_term(CodeUnit* code, Term* func, const TermList& inputs);

// Convenience function for creating a term with no inputs
Term* create_term(CodeUnit* code, Term* function);

Term* create_term(CodeUnit* code, Term* func, const TermList& inputs);
Term* create_constant(CodeUnit* code, Term* type);
void set_input(CodeUnit* code, Term* term, int index, Term* input);
void set_inputs(CodeUnit* code, Term* term, const TermList& inputs);
void bind_name(CodeUnit* code, Term* term, string name);

class CodeUnitTermIterator : public TermIterator
{
public:
    virtual Term* next()
    {
        _index += 1;
        return get();
    }
    virtual Term* get() const
    {
        if (more()) return _code_unit->all_terms[_index];
    }
    virtual bool more() const
    {
        return _index < _code_unit->all_terms.size();
    }

    CodeUnitTermIterator(CodeUnit* code)
      : _index(0), _code_unit(code)
    {}

private:
    int _index;
    CodeUnit *_code_unit;
};

}

#endif
