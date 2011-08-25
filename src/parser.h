// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

struct ParserCxt {
    std::string pendingRebind;
};

struct ParseResult {
    Term* term;
    std::string identifierName;

    ParseResult() : term(NULL) {}
    explicit ParseResult(Term* t) : term(t) {}
    explicit ParseResult(Term* t, std::string s) : term(t), identifierName(s) {}
    bool isIdentifier() { return identifierName != ""; }
};

typedef ParseResult (*ParsingStep)(Branch& branch, TokenStream& tokens, ParserCxt* context);

TermPtr compile(Branch& branch, ParsingStep step, std::string const& input);
TermPtr evaluate(Branch& branch, ParsingStep step, std::string const& input);

Term* evaluate(Branch& branch, std::string const& input);

// Parsing steps:
ParseResult statement_list(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult comment(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult blank_line(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult function_decl(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult type_decl(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult anonymous_type_decl(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult if_block(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult switch_block(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult for_block(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult do_once_block(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult stateful_value_decl(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult expression_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult include_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult return_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult discard_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult break_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult continue_statement(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult name_binding_expression(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult expression(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult infix_expression(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult infix_expression_nested(Branch& branch, TokenStream& tokens, ParserCxt* context,
        int precedence);
ParseResult unary_expression(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult function_call(Branch& branch, ParseResult head, TokenStream& tokens, ParserCxt* context);
ParseResult atom_with_subscripts(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult atom(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_integer(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_hex(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_float(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_string(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_bool(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_null(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_color(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult literal_list(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult plain_branch(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult namespace_block(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult unknown_identifier(Branch& branch, std::string const& name);
ParseResult identifier(Branch& branch, TokenStream& tokens, ParserCxt* context);
ParseResult identifier_with_rebind(Branch& branch, TokenStream& tokens, ParserCxt* context);

// Helper functions:
void consume_branch(Branch& branch, TokenStream& tokens, ParserCxt* context);
void consume_branch_with_significant_indentation(Branch& branch, TokenStream& tokens,
        ParserCxt* context, Term* parent);
void consume_branch_with_braces(Branch& branch, TokenStream& tokens, ParserCxt* context,
        Term* parentTerm);
bool lookahead_match_whitespace_statement(TokenStream& tokens);
bool lookahead_match_comment_statement(TokenStream& tokens);
bool lookahead_match_leading_name_binding(TokenStream& tokens);
bool lookahead_match_rebind_argument(TokenStream& tokens);
Term* find_lexpr_root(Term* term);

// Check if 'target' is a namespace access; if so, we'll return the original
// term that it accesses. If not, we'll just return 'target'.
//Term* statically_resolve_namespace_access(Term* target);

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void set_starting_source_location(Term* term, int start, TokenStream& tokens);
void set_source_location(Term* term, int start, TokenStream& tokens);
Term* find_and_apply(Branch& branch, std::string const& functionName,
        TermList const& inputs);

// Find a type with the given name, looking in this branch. If the name isn't found,
// we'll return a call to unknown_type()
Term* find_type(Branch& branch, std::string const& name);

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
ParseResult compile_error_for_line(Branch& branch, TokenStream &tokens, int start,
        std::string const& message="");

// Consume the nearby line, convert 'existing' into a compile-error term, and
// return it.
ParseResult compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message="");

// Helper functions:
bool is_infix_operator_rebinding(std::string const& infix);
std::string possible_whitespace(TokenStream& tokens);
std::string possible_newline(TokenStream& tokens);
std::string possible_whitespace_or_newline(TokenStream& tokens);
bool is_statement_ending(int t);
std::string possible_statement_ending(TokenStream& tokens);
bool is_multiline_block(Term* term);

int get_number_of_decimal_figures(std::string const& str);

} // namespace parser


} // namespace circa
