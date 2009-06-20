#ifndef CIRCA_PARSER_UTIL_INCLUDED
#define CIRCA_PARSER_UTIL_INCLUDED

#include "common_headers.h"

namespace circa {

namespace tokenizer { struct Token; }

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void include_location(Term* term, tokenizer::Token& tok);
void set_source_location(Term* term, int start, TokenStream& tokens);
Term* find_and_apply(Branch& branch, std::string const& functionName, RefList inputs);
void recursively_mark_terms_as_occuring_inside_an_expression(Term* term);

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

// Mark the given term as hidden from source reproduction.
void source_set_hidden(Term* term, bool hidden);

// Consume tokens starting at 'start' and ending at something which might
// be the end of the statement. Return line as string. This should probably
// only be used for handling parse errrors.
// If 'positionRecepient' is not NULL then we will include the positions of the
// consumed tokens in its syntax hints.
std::string consume_line(TokenStream &tokens, int start, Term* positionRecepient=NULL);

// Consume the nearby line, return a newly created compile-error term.
Term* compile_error_for_line(Branch& branch, TokenStream &tokens, int start);

// Consume the nearby line, convert 'existing' into a compile-error term, and
// return it.
Term* compile_error_for_line(Term* existing, TokenStream &tokens, int start);

// Helper functions:
bool is_infix_operator_rebinding(std::string const& infix);
std::string possible_whitespace(TokenStream& tokens);
std::string possible_newline(TokenStream& tokens);
std::string possible_whitespace_or_newline(TokenStream& tokens);
std::string possible_statement_ending(TokenStream& tokens);
void consume_branch_until_end(Branch& branch, TokenStream& tokens);

} // namespace circa

#endif
