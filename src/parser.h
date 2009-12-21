// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_PARSER_INCLUDED
#define CIRCA_PARSER_INCLUDED

#include "common_headers.h"

#include "token_stream.h"

namespace circa {
namespace parser {

enum BranchSyntax {
    BRANCH_SYNTAX_UNDEF=0,
    BRANCH_SYNTAX_COLON=1,
    BRANCH_SYNTAX_IMPLICIT_BEGIN=2,
    BRANCH_SYNTAX_BEGIN=3,
    BRANCH_SYNTAX_BRACE=4
};

typedef Term* (*ParsingStep)(Branch& branch, TokenStream& tokens);

Ref compile(Branch* branch, ParsingStep step, std::string const& input);
Ref evaluate(Branch& branch, ParsingStep step, std::string const& input);

// Parsing steps:
Term* statement_list(Branch& branch, TokenStream& tokens);
Term* statement(Branch& branch, TokenStream& tokens);
Term* comment(Branch& branch, TokenStream& tokens);
Term* blank_line(Branch& branch, TokenStream& tokens);
Term* function_decl(Branch& branch, TokenStream& tokens);
Term* type_decl(Branch& branch, TokenStream& tokens);
Term* anonymous_type_decl(Branch& branch, TokenStream& tokens);
Term* if_block(Branch& branch, TokenStream& tokens);
Term* for_block(Branch& branch, TokenStream& tokens);
Term* do_once_block(Branch& branch, TokenStream& tokens);
Term* stateful_value_decl(Branch& branch, TokenStream& tokens);
Term* expression_statement(Branch& branch, TokenStream& tokens);
Term* return_statement(Branch& branch, TokenStream& tokens);
Term* include_statement(Branch& branch, TokenStream& tokens);
Term* discard_statement(Branch& branch, TokenStream& tokens);
Term* infix_expression(Branch& branch, TokenStream& tokens);
Term* infix_expression_nested(Branch& branch, TokenStream& tokens, int precedence);
Term* unary_expression(Branch& branch, TokenStream& tokens);
Term* function_call(Branch& branch, Term* function, RefList const& inputs);
Term* subscripted_atom(Branch& branch, TokenStream& tokens);
Term* qualified_identifier(Branch& branch, TokenStream& tokens);
Term* atom(Branch& branch, TokenStream& tokens);
Term* literal_integer(Branch& branch, TokenStream& tokens);
Term* literal_hex(Branch& branch, TokenStream& tokens);
Term* literal_float(Branch& branch, TokenStream& tokens);
Term* literal_string(Branch& branch, TokenStream& tokens);
Term* literal_bool(Branch& branch, TokenStream& tokens);
Term* literal_color(Branch& branch, TokenStream& tokens);
Term* literal_list(Branch& branch, TokenStream& tokens);
Term* plain_branch(Branch& branch, TokenStream& tokens);
Term* namespace_block(Branch& branch, TokenStream& tokens);
Term* identifier_or_lexpr(Branch& branch, TokenStream& tokens);

Term* constant_fold_lexpr(Term* call);

// Helper functions:
void consume_branch(Branch& branch, TokenStream& tokens);
void consume_branch_with_significant_indentation(Branch& branch, TokenStream& tokens);

} // namespace parser
} // namespace circa

#endif
