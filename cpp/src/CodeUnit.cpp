
#include "Builtins.h"
#include "CodeUnit.h"
#include "CommonHeaders.h"
#include "Function.h"
#include "Type.h"

namespace codeunit {

static Term* _new_term(CodeUnit* code);

Term* bootstrap_empty_term(CodeUnit* code)
{
    return _new_term(code);
}

Term* get_term(CodeUnit* code, Term* func, const TermList& inputs)
{
    // todo: check to reuse an existing term
    return create_term(code, func, inputs);
}

// Convenience function for creating a term with no inputs
Term* create_term(CodeUnit* code, Term* func)
{
    return create_term(code, func, TermList());
}

Term* create_term(CodeUnit* code, Term* func, const TermList& inputs)
{
    printf("create_term enter\n");
    if (code == NULL) {
        printf("ERROR: create_term called with code == NULL\n");
        return NULL;
    }

    if (func == NULL) {
        printf("ERROR: create_term called with func == NULL\n");
        return NULL;
    }

    Term* new_term = _new_term(code);
    new_term->function = func;
    set_inputs(code, new_term, inputs);

    Term* new_terms_type = function::output_type(func);

    if (new_terms_type == NULL) {
        printf("ERROR: function %s has null type\n", func->to_string().c_str());
        return NULL;
    }

    type::call_initialize_data(function::output_type(func), new_term);

    printf("create_term exit\n");

    return new_term;
}

Term* create_constant(CodeUnit* code, Term* type)
{
    // Fetch the constant-function for this type
    Term* constFunc = get_term(code, builtins::CONST_GENERATOR, TermList(type));

    if (constFunc == NULL) {
       throw std::exception();
    }

    return create_term(code, constFunc, TermList());
}

void set_input(CodeUnit* code, Term* term, int index, Term* input)
{
    term->inputs.set(index, input);
}

void set_inputs(CodeUnit* code, Term* term, const TermList& inputs)
{
    term->inputs = inputs;
}

void bind_name(CodeUnit* code, Term* term, string name)
{
    //code->term_namespace[name] = term;
}

Term* _new_term(CodeUnit* code)
{
    Term* term = new Term;
    code->all_terms.push_back(term);
    return term;
}

} // namespace codeunit
