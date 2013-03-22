// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void format_block_source(caValue* source, Block* block, Term* format=NULL);
std::string unformat_rich_source(caValue* source);

void format_term_source(caValue* source, Term* term);
void format_term_source_default_formatting(caValue* source, Term* term);
void format_term_source_normal(caValue* source, Term* term);
void block_to_code_lines(Block* block, caValue* out);

// Formats source for the given input, as used by the term.
void format_source_for_input(caValue* source, Term* term, int inputIndex);
void format_source_for_input(caValue* source, Term* term, int inputIndex,
        const char* defaultPre, const char* defaultPost);

void format_name_binding(caValue* source, Term* term);

void append_phrase(caValue* source, const char* str, Term* term, Symbol type);
// Convenient overload:
void append_phrase(caValue* source, std::string const& str, Term* term, Symbol type);

std::string get_block_source_text(Block* block);
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
void set_input_syntax_hint(Term* term, int index, const char* field, caValue* value);

// Mark the given term as hidden from source reproduction.
void hide_from_source(Term* term);

} // namespace circa
