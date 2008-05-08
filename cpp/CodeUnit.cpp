
#include "Builtins.h"
#include "CodeUnit.h"
#include "CommonHeaders.h"
#include "Errors.h"
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
    if (code == NULL) {
        INTERNAL_ERROR("create_term called with code == NULL\n");
    }

    if (func == NULL) {
        INTERNAL_ERROR("create_term called with func == NULL\n");
    }

    Term* new_term = _new_term(code);
    new_term->function = func;
    set_inputs(code, new_term, inputs);

    Term* new_terms_type = function::output_type(func);

    if (new_terms_type == NULL) {
        INTERNAL_ERROR(string("ERROR: function")+func->to_string().c_str()+ "has null type");
    }

    // Initialize data
    type::call_initialize_data(function::output_type(func), new_term);

    // In some situations, call evaluate immediately
    if (function::pure_function(func) && !inputs.any_need_update()) {
        new_term->evaluate();
    }

    return new_term;
}

Term* create_constant(CodeUnit* code, Term* type)
{
    // Fetch the constant-function for this type
    Term* constFunc = get_term(code, builtins::CONST_GENERATOR, TermList(type));

    if (constFunc == NULL) {
        INTERNAL_ERROR("constFunc is NULL");
    }

    constFunc->evaluate();

    return create_term(code, constFunc, TermList());
}

void set_input(CodeUnit* code, Term* term, int index, Term* input)
{
    if (code == NULL) INTERNAL_ERROR("code is NULL");
    if (term == NULL) INTERNAL_ERROR("term is NULL");
    
    term->inputs.set(index, input);
}

void set_inputs(CodeUnit* code, Term* term, const TermList& inputs)
{
    if (code == NULL) INTERNAL_ERROR("code is NULL");
    if (term == NULL) INTERNAL_ERROR("term is NULL");
    term->inputs = inputs;
}

void bind_name(CodeUnit* code, Term* term, string name)
{
    //code->term_namespace[name] = term;
}

Term* _new_term(CodeUnit* code)
{
    if (code == NULL) INTERNAL_ERROR("code is NULL");
    Term* term = new Term();
    code->all_terms.push_back(term);
    return term;
}

} // namespace codeunit
