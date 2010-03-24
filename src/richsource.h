// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_RICHSOURCE_INCLUDED
#define CIRCA_RICHSOURCE_INCLUDED

#include "common_headers.h"

#include "list_t.h"

namespace circa {

namespace richsource {
    enum PhraseType {
        UNDEFINED=0,
        COMMENT,
        NEWLINE,
        WHITESPACE,
        BOOL_VALUE,
        INT_VALUE,
        FLOAT_VALUE,
        FUNCTION_CALL,
        INFIX_OPERATOR,
        PREFIX_OPERATOR,
        FUNCTION_NAME,
        LPAREN,
        RPAREN,
        FOR,
        IF,
        END
    };
}

struct RichSource
{
    List _phrases;
};

void format_branch_source(Branch& branch, RichSource* source);

void append_phrase(RichSource* source, const char* str, Term* term, richsource::PhraseType type);
void append_leading_name_binding(RichSource* source, Term* term);

}

#endif
