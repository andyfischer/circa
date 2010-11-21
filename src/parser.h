// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include "token_stream.h"

namespace circa {
namespace parser {

enum BranchSyntax {
    BRANCH_SYNTAX_UNDEF=0,
    BRANCH_SYNTAX_COLON=1,
    BRANCH_SYNTAX_IMPLICIT_BEGIN=2, // deprecated
    BRANCH_SYNTAX_BEGIN=3,          // deprecated
    BRANCH_SYNTAX_BRACE=4,
    BRANCH_SYNTAX_DO=5
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
Term* include_statement(Branch& branch, TokenStream& tokens);
Term* return_statement(Branch& branch, TokenStream& tokens);
Term* discard_statement(Branch& branch, TokenStream& tokens);
Term* bindable_expression(Branch& branch, TokenStream& tokens);
Term* infix_expression(Branch& branch, TokenStream& tokens);
Term* infix_expression_nested(Branch& branch, TokenStream& tokens, int precedence);
Term* unary_expression(Branch& branch, TokenStream& tokens);
Term* function_call(Branch& branch, Term* function, TokenStream& tokens);
Term* subscripted_atom(Branch& branch, TokenStream& tokens);
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
Term* identifier(Branch& branch, TokenStream& tokens);
Term* identifier(Branch& branch, TokenStream& tokens, std::string& idStrOut);
Term* identifier_with_rebind(Branch& branch, TokenStream& tokens);

// Helper functions:
void consume_branch(Branch& branch, TokenStream& tokens);
void consume_branch_with_significant_indentation(Branch& branch, TokenStream& tokens);
bool lookahead_match_whitespace_statement(TokenStream& tokens);
bool lookahead_match_comment_statement(TokenStream& tokens);
bool lookahead_match_leading_name_binding(TokenStream& tokens);
Term* find_lexpr_root(Term* term);

// Check if 'target' is a namespace access; if so, we'll return the original
// term that it accesses. If not, we'll just return 'target'.
Term* statically_resolve_namespace_access(Term* target);

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void set_source_location(Term* term, int start, TokenStream& tokens);
Term* find_and_apply(Branch& branch, std::string const& functionName, RefList inputs);

// Find a type with the given name, looking in this branch. If the name isn't found,
// we'll return a call to unknown_type()
Term* find_type(Branch& branch, std::string const& name);

// Find a function with the given name, looking in this branch. If the name isn't found,
// we'll return unknown_function()
Term* find_function(Branch& branch, std::string const& name);

void push_pending_rebind(Branch& branch, std::string const& name);
std::string pop_pending_rebind(Branch& branch);

// Does various cleanup work on a branch that has just been used by a parsing step.
// This should be done after parsing.
void post_parse_branch(Branch& branch);

// Consume tokens starting at 'start' and ending at something which might
// be the end of the statement. Return line as string. This should probably
// only be used for handling parse errrors.
// If 'positionRecepient' is not NULL then we will include the positions of the
// consumed tokens in its syntax hints.
std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient=NULL);

// Consume the nearby line, return a newly created compile-error term.
Term* compile_error_for_line(Branch& branch, TokenStream &tokens, int start,
        std::string const& message="");

// Consume the nearby line, convert 'existing' into a compile-error term, and
// return it.
Term* compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message="");

// Helper functions:
bool is_infix_operator_rebinding(std::string const& infix);
std::string possible_whitespace(TokenStream& tokens);
std::string possible_newline(TokenStream& tokens);
std::string possible_whitespace_or_newline(TokenStream& tokens);
std::string possible_statement_ending(TokenStream& tokens);

int get_number_of_decimal_figures(std::string const& str);

} // namespace parser


} // namespace circa
