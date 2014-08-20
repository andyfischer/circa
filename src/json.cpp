// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "list.h"
#include "names.h"
#include "parser.h"
#include "string_type.h"
#include "tagged_value.h"
#include "token.h"
#include "type.h"

namespace circa {

struct ParseContext
{
    TokenStream* tokens;
    Value* out;
};

static void drop_whitespace(TokenStream* tokens)
{
    while (tokens->nextIs(tok_Whitespace))
        tokens->consume();
}

static void parse_value(TokenStream* tokens, Value* out)
{
    // ignore leading whitespace
    drop_whitespace(tokens);

    if (tokens->finished()) {
        set_string(out, "unexpected end of string");

    } else if (tokens->nextIs(tok_Integer)) {
        Value nextStr;
        tokens->getNextStr(&nextStr);
        set_int(out, atoi(as_cstring(&nextStr)));
        tokens->consume(tok_Integer);
    } else if (tokens->nextIs(tok_Float)) {
        Value nextStr;
        tokens->getNextStr(&nextStr);
        set_float(out, (float) atof(as_cstring(&nextStr)));
        tokens->consume(tok_Float);
    } else if (tokens->nextIs(tok_String)) {
        tokens->consumeStr(out, tok_String);
        string_unquote_and_unescape(out);
    } else if (tokens->nextIs(tok_LSquare)) {
        tokens->consume(tok_LSquare);
        drop_whitespace(tokens);

        set_list(out, 0);

        while (!tokens->nextIs(tok_RSquare) && !tokens->finished()) {
            Value* element = list_append(out);
            parse_value(tokens, element);

            if (tokens->nextIs(tok_Comma))
                tokens->consume(tok_Comma);
            drop_whitespace(tokens);
        }

        if (!tokens->finished())
            tokens->consume(tok_RSquare);
    } else if (tokens->nextIs(tok_True)) {
        set_bool(out, true);
        tokens->consume();
    } else if (tokens->nextIs(tok_False)) {
        set_bool(out, false);
        tokens->consume();
    } else if (tokens->nextIs(tok_Minus)) {
        tokens->consume(tok_Minus);

        parse_value(tokens, out);

        if (is_int(out)) {
            set_int(out, -1 * as_int(out));
        } else if (is_float(out)) {
            set_float(out, -1 * as_float(out));
        } else {
            set_string(out, "error, minus sign must preceed number");
        }

    } else {
        set_string(out, "unrecognized token: ");
        Value next;
        tokens->getNextStr(&next);
        string_append(out, &next);
        tokens->consume();
    }

    // ignore trailing whitespace
    drop_whitespace(tokens);
}

void json_parse(const char* in, Value* out)
{
    Value inStr;
    set_string(&inStr, in);
    TokenStream tokens(&inStr);
    parse_value(&tokens, out);
}

void json_write(Value* in, Value* out)
{
    if (!is_string(out))
        set_string(out, "");

    // For certain types, just use to_string
    if (is_int(in) || is_float(in) || is_bool(in)) {
        string_append(out, in);
    } else if (is_list(in)) {
        set_string(out, "");
        string_append(out, "[");

        for (int i=0; i < list_length(in); i++) {
            if (i > 0)
                string_append(out, ", ");

            Value elementStr;
            json_write(list_get(in, i), &elementStr);
            string_append(out, &elementStr);
        }

        string_append(out, "]");
    } else if (is_string(in)) {
        copy(in, out);
        string_quote_and_escape(out);
    } else {
        set_string(out, "error: no string repr for type ");
        string_append(out, as_cstring(&in->value_type->name));
    }
}

CIRCA_EXPORT void circa_parse_json(Value* in, Value* out)
{
    json_parse(as_cstring(in), out);
}

CIRCA_EXPORT void circa_to_json(Value* in, Value* out)
{
    json_write(in, out);
}

} // namespace circa
