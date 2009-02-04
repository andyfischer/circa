// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

namespace circa {

namespace ast {
    class FunctionCall;
    class Infix;
    class LiteralString;
    class LiteralFloat;
    class LiteralInteger;
    class StatefulValueDeclaration;
}

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        ReferenceList inputs);

bool push_is_inside_expression(Branch& branch, bool value);
void pop_is_inside_expression(Branch& branch, bool value);
bool is_inside_expression(Branch& branch);
void push_pending_rebind(Branch& branch, std::string const& name);
std::string pop_pending_rebind(Branch& branch);
void remove_compilation_attrs(Branch& branch);

Term* create_comment(Branch& branch, std::string const& text);
Term* create_literal_string(Branch& branch, ast::LiteralString& ast);
Term* create_literal_float(Branch& branch, ast::LiteralFloat& ast);
Term* create_literal_integer(Branch& branch, ast::LiteralInteger& ast);
Term* create_dot_concatenated_call(Branch &context, ast::Infix& ast);
Term* create_arrow_concatenated_call(Branch &context, ast::Infix& ast);
Term* create_feedback_call(Branch &context, ast::Infix& ast);
Term* create_infix_call(Branch &context, ast::Infix& ast);
Term* create_function_call(Branch &context, ast::FunctionCall& ast);
Term* create_stateful_value_declaration(Branch &context,
        ast::StatefulValueDeclaration& ast);

} // namespace circa

#endif
