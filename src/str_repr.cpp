// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "parser.h"
#include "tagged_value.h"
#include "token.h"

using namespace circa;

struct ParseContext
{
    TokenStream* tokens;
    caValue* out;
};

static void drop_whitespace(TokenStream* tokens)
{
    while (tokens->nextIs(TK_WHITESPACE))
        tokens->consume();
}

static void parse_value(TokenStream* tokens, caValue* out)
{
    // ignore leading whitespace
    drop_whitespace(tokens);

    if (tokens->nextIs(TK_INTEGER)) {
        circa_set_int(out, atoi(tokens->nextStr().c_str()));
        tokens->consume(TK_INTEGER);
    } else if (tokens->nextIs(TK_FLOAT)) {
        circa_set_float(out, atof(tokens->nextStr().c_str()));
        tokens->consume(TK_FLOAT);
    } else if (tokens->nextIs(TK_STRING)) {
        std::string s = tokens->nextStr();
        parser::unquote_and_unescape_string(s.c_str(), (caValue*) out);
        tokens->consume(TK_STRING);
    } else if (tokens->nextIs(TK_LBRACKET)) {
        tokens->consume(TK_LBRACKET);
        drop_whitespace(tokens);

        circa_set_list(out, 0);

        while (!tokens->nextIs(TK_RBRACKET) && !tokens->finished()) {
            caValue* element = circa_list_append(out);
            parse_value(tokens, element);

            if (tokens->nextIs(TK_COMMA))
                tokens->consume(TK_COMMA);
            drop_whitespace(tokens);
        }

        if (!tokens->finished())
            tokens->consume(TK_RBRACKET);
    } else if (tokens->nextIs(TK_TRUE)) {
        circa_set_bool(out, true);
        tokens->consume();
    } else if (tokens->nextIs(TK_FALSE)) {
        circa_set_bool(out, false);
        tokens->consume();
    } else if (tokens->nextIs(TK_MINUS)) {
        tokens->consume(TK_MINUS);

        parse_value(tokens, out);

        if (circa_is_int(out)) {
            circa_set_int(out, -1 * circa_as_int(out));
        } else if (circa_is_float(out)) {
            circa_set_float(out, -1 * circa_as_float(out));
        } else {
            circa_set_string(out, "error, minus sign must preceed number");
        }

    } else {
        circa_set_string(out, "unrecognized token: ");
        circa_string_append(out, tokens->nextStr().c_str());
        tokens->consume();
    }

    // ignore trailing whitespace
    drop_whitespace(tokens);
}

extern "C" void circa_parse_string(const char* str, caValue* out)
{
    TokenStream tokens(str);
    parse_value(&tokens, out);
}
