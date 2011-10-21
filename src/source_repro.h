// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "types/list.h"

namespace circa {

namespace phrase_type {
    // Token enumerations are valid as phrase types. This list has some addtional
    // types which are not tokens.
    const int UNDEFINED = 200;
    const int INFIX_OPERATOR = 201;
    const int FUNCTION_NAME = 202;
    const int TYPE_NAME = 203;
    const int TERM_NAME = 204;
    const int KEYWORD = 205;
    const int WHITESPACE = 206;

    const int UNKNOWN_IDENTIFIER = 220;
}

struct StyledSource
{
    // Each phrase has type:
    // Phrase {
    //   string text
    //   Ref term
    //   int phrase_type
    // }
    
    List _phrases;

    std::string toString();
};

void format_branch_source(StyledSource* source, Branch* branch, Term* format=NULL);
std::string unformat_rich_source(StyledSource* source);

void format_term_source(StyledSource* source, Term* term);
void format_term_source_default_formatting(StyledSource* source, Term* term);

// Formats source for the given input, as used by the term.
void format_source_for_input(StyledSource* source, Term* term, int inputIndex);
void format_source_for_input(StyledSource* source, Term* term, int inputIndex,
        const char* defaultPre, const char* defaultPost);

void format_name_binding(StyledSource* source, Term* term);

void append_phrase(StyledSource* source, const char* str, Term* term, int type);
// Convenient overload:
void append_phrase(StyledSource* source, std::string const& str, Term* term, int type);

std::string get_branch_source_text(Branch* branch);
std::string get_term_source_text(Term* term);
std::string get_input_source_text(Term* term, int index);

bool should_print_term_source_line(Term* term);
bool is_hidden(Term* term);
int get_first_visible_input_index(Term* term);

std::string get_input_syntax_hint(Term* term, int index, const char* field);
std::string get_input_syntax_hint_optional(Term* term, int index, const char* field,
        std::string const& defaultValue);
void set_input_syntax_hint(Term* term, int index, const char* field,
        std::string const& value);

// Mark the given term as hidden from source reproduction.
void hide_from_source(Term* term);

} // namespace circa
