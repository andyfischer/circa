#ifndef CIRCA_PARSER_UTIL_INCLUDED
#define CIRCA_PARSER_UTIL_INCLUDED

#include "common_headers.h"

namespace circa {

namespace tokenizer { class Token; }

void prepend_whitespace(Term* term, std::string const& whitespace);
void append_whitespace(Term* term, std::string const& whitespace);
void include_location(Term* term, tokenizer::Token& tok);
Term* find_and_apply(Branch& branch, std::string const& functionName, RefList inputs);
void recursively_mark_terms_as_occuring_inside_an_expression(Term* term);
Term* find_type(Branch& branch, std::string const& name);
Term* find_function(Branch& branch, std::string const& name);
void push_pending_rebind(Branch& branch, std::string const& name);
std::string pop_pending_rebind(Branch& branch);
void remove_compilation_attrs(Branch& branch);
void wrap_up_branch(Branch& branch);

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

} // namespace circa

#endif
