// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Block;
struct TokenStream;
struct ParserCxt;

struct ParseResult {
    Term* term;

    // When the parser finds an identifier to an existing term, the ParseResult has
    // a non-empty identifierName, so that the calling function can tell what happened.
    // The identifierName should only be filled in if the parse step did *not* create
    // a new term.
    std::string identifierName;

    // For an identifier term, whether the identifier has a @ decoration.
    bool identifierRebind;

    ParseResult() : term(NULL), identifierRebind(false) {}
    explicit ParseResult(Term* t) : term(t), identifierRebind(false) {}
    explicit ParseResult(Term* t, std::string s) : term(t), identifierName(s), identifierRebind(false) {}
    bool isIdentifier() { return identifierName != ""; }
};

typedef ParseResult (*ParsingStep)(Block* block, TokenStream& tokens, ParserCxt* context);

Term* parse(Block* block, ParsingStep step, Value* input);
Term* parse(Block* block, ParsingStep step, const char* input);

// Parsing steps:
ParseResult parse_statement_list(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult parse_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult comment(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult blank_line(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult function_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult anon_function_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult struct_decl(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult if_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult switch_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult case_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult else_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult require_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult while_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult package_statement(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult for_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult state_decl(Block* block, TokenStream& tokens, ParserCxt* context);
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
ParseResult method_call(Block* block, TokenStream& tokens, ParserCxt* context, ParseResult lhs);
ParseResult function_call(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult dot_symbol(Block* block, TokenStream& tokens, ParserCxt* context, ParseResult lhs);
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
ParseResult literal_symbol(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult closure_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult section_block(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult unknown_identifier(Block* block, std::string const& name);
ParseResult identifier(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult identifier_possibly_null(Block* block, TokenStream& tokens, ParserCxt* context);
ParseResult identifier_with_rebind(Block* block, TokenStream& tokens, ParserCxt* context);

// Helper functions:
void consume_block(Block* block, TokenStream& tokens, ParserCxt* context);
void consume_block_with_significant_indentation(Block* block, TokenStream& tokens,
        ParserCxt* context, Term* parent);
void consume_block_with_braces(Block* block, TokenStream& tokens, ParserCxt* context,
        Term* parentTerm);
bool lookahead_match_whitespace_statement(TokenStream& tokens);
bool lookahead_match_comment_statement(TokenStream& tokens);
bool lookahead_match_rebind_argument(TokenStream& tokens);
bool lookahead_match_anon_function(TokenStream& tokens);
Term* find_lexpr_root(Term* term);

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void set_starting_source_location(Term* term, int start, TokenStream& tokens);
void set_source_location(Term* term, int start, TokenStream& tokens);
Term* find_and_apply(Block* block, std::string const& functionName,
        TermList const& inputs);

// Helper functions
bool is_infix_operator_rebinding(int match);
std::string possible_whitespace(TokenStream& tokens);
std::string possible_newline(TokenStream& tokens);
std::string possible_whitespace_or_newline(TokenStream& tokens);
bool is_statement_ending(int t);
std::string possible_statement_ending(TokenStream& tokens);
bool is_multiline_block(Term* term);

int get_number_of_decimal_figures(std::string const& str);

} // namespace circa
