// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_RICHSOURCE_INCLUDED
#define CIRCA_RICHSOURCE_INCLUDED

#include "common_headers.h"

#include "list_t.h"

namespace circa {

namespace phrase_type {
    // Token enumerations are valid as phrase types.
    // This list has some types which are not tokens.
    const int UNDEFINED = 200;
    const int INFIX_OPERATOR = 201;
    const int FUNCTION_NAME = 202;
    const int TYPE_NAME = 203;
    const int TERM_NAME = 204;
}

struct RichSource
{
    List _phrases;

    std::string toString();
};

void append_branch_source(RichSource* source, Branch& branch);

void append_phrase(RichSource* source, const char* str, Term* term, int type);
// Convenient overload:
void append_phrase(RichSource* source, std::string const& str, Term* term, int type);

void append_leading_name_binding(RichSource* source, Term* term);
void append_source_for_input(RichSource* source, Term* term, int inputIndex);
void append_term_source(RichSource* source, Term* term);

std::string unformat_rich_source(RichSource* source);

}

#endif
