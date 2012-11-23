// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Block;
struct TokenStream;

namespace parser {

enum BlockSyntax {
    BLOCK_SYNTAX_UNDEF=0,
    BLOCK_SYNTAX_COLON=1,
    BLOCK_SYNTAX_IMPLICIT_BEGIN=2, // deprecated
    BLOCK_SYNTAX_BEGIN=3,          // deprecated
    BLOCK_SYNTAX_BRACE=4,
    BLOCK_SYNTAX_DO=5
};

struct ParserCxt {
    std::string pendingRebind;

    // Number of open parenthesis on the current expression. This affects whether we'll
    // consume newlines as part of an expression).
    int openParens;

    ParserCxt()
      : openParens(0)
    {}
};

struct ParseResult {
    Term* term;

    // When the parser finds an identifier to an existing term, the ParseResult has
    // a non-empty identifierName, so that the calling function can tell what happened.
    // The identifierName should only be filled in if the parse step did *not* create
    // a new term.
    std::string identifierName;

    ParseResult() : term(NULL) {}
    explicit ParseResult(Term* t) : term(t) {}
    explicit ParseResult(Term* t, std::string s) : term(t), identifierName(s) {}
    bool isIdentifier() { return identifierName != ""; }
};

typedef ParseResult (*ParsingStep)(Block* block, TokenStream& tokens, ParserCxt* context);

Term* compile(Block* block, ParsingStep step, std::string const& input);
Term* evaluate(Block* block, ParsingStep step, std::string const& input);

Term* evaluate(Block* block, std::string const& input);

// Parsing steps:
ParseResult statement_list(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult comment(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult blank_line(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult function_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult type_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult if_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult switch_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult case_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult require_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult package_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult for_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult stateful_value_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult expression_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult include_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult import_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult return_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult discard_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult break_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult continue_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult name_binding_expression(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult expression(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult infix_expression(Block* block, TokenStream& tokens, ParserCxt* context,
        int minimumPrecedence);
ParseResult unary_expression(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult function_call(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult atom_with_subscripts(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult atom(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_integer(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_hex(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_float(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_string(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_bool(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_null(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_color(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_list(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult literal_name(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult closure_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult namespace_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult unknown_identifier(Block* block, std::string const& name);
ParseResult identifier(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult identifier_with_rebind(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult identifier_no_create(Block* block, TokenStream& tokens, ParserCxt* context);

// Helper functions:
void consume_block(Block* block, TokenStream& tokens, ParserCxt* context);
void consume_block_with_significant_indentation(Block* block, TokenStream& tokens,
        ParserCxt* context, Term* parent);
void consume_block_with_braces(Block* block, TokenStream& tokens, ParserCxt* context,
        Term* parentTerm);
bool lookahead_match_whitespace_statement(TokenStream& tokens);
bool lookahead_match_comment_statement(TokenStream& tokens);
bool lookahead_match_rebind_argument(TokenStream& tokens);
Term* find_lexpr_root(Term* term);

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void set_starting_source_location(Term* term, int start, TokenStream& tokens);
void set_source_location(Term* term, int start, TokenStream& tokens);
Term* find_and_apply(Block* block, std::string const& functionName,
        TermList const& inputs);

// Consume tokens starting at 'start' and ending at something which might
// be the end of the statement. Return line as string. This should probably
// only be used for handling parse errrors.
// If 'positionRecepient' is not NULL then we will include the positions of the
// consumed tokens in its syntax hints.
std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient=NULL);

// Consume the nearby line, return a newly created compile-error term.
ParseResult compile_error_for_line(Block* block, TokenStream &tokens, int start,
        std::string const& message="");

// Consume the nearby line, convert 'existing' into a compile-error term, and
// return it.
ParseResult compile_error_for_line(Term* existing, TokenStream &tokens, int start,
        std::string const& message="");

// Helper functions:
bool is_infix_operator_rebinding(int match);
std::string possible_whitespace(TokenStream& tokens);
std::string possible_newline(TokenStream& tokens);
std::string possible_whitespace_or_newline(TokenStream& tokens);
bool is_statement_ending(int t);
std::string possible_statement_ending(TokenStream& tokens);
bool is_multiline_block(Term* term);

int get_number_of_decimal_figures(std::string const& str);
void unquote_and_unescape_string(const char* input, caValue* out);
void quote_and_escape_string(const char* input, caValue* out);

} // namespace parser

} // namespace circa
