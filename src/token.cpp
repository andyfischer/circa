// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "token.h"

namespace circa {

const char* get_token_text(int match)
{
    switch (match) {
        case tok_LParen: return "(";
        case tok_RParen: return ")";
        case tok_LBrace: return "{";
        case tok_RBrace: return "}";
        case tok_LBracket: return "[";
        case tok_RBracket: return "]";
        case tok_Comma: return ",";
        case tok_At: return "@";
        case tok_Identifier: return "IDENTIFIER";
        case tok_Name: return "NAME";
        case tok_Integer: return "INTEGER";
        case tok_HexInteger: return "HEX_INTEGER";
        case tok_Float: return "FLOAT";
        case tok_String: return "STRING";
        case tok_Color: return "COLOR";
        case tok_Comment: return "COMMENT";
        case tok_Dot: return ".";
        case tok_DotAt: return ".@";
        case tok_Star: return "*";
        case tok_Question: return "?";
        case tok_Slash: return "/";
        case tok_DoubleSlash: return "//";
        case tok_Plus: return "+";
        case tok_Minus: return "-";
        case tok_LThan: return "<";
        case tok_LThanEq: return "<=";
        case tok_GThan: return ">";
        case tok_GThanEq: return ">=";
        case tok_Percent: return "%";
        case tok_Colon: return ":";
        case tok_DoubleColon: return "::";
        case tok_DoubleEquals: return "==";
        case tok_NotEquals: return "!=";
        case tok_Equals: return "=";
        case tok_PlusEquals: return "+=";
        case tok_MinusEquals: return "-=";
        case tok_StarEquals: return "*=";
        case tok_SlashEquals: return "/=";
        case tok_ColonEquals: return ":=";
        case tok_RightArrow: return "->";
        case tok_LeftArrow: return "<-";
        case tok_Ampersand: return "&";
        case tok_DoubleAmpersand: return "&&";
        case tok_DoubleVerticalBar: return "||";
        case tok_Semicolon: return ";";
        case tok_TwoDots: return "..";
        case tok_Ellipsis: return "...";
        case tok_TripleLThan: return "<<<";
        case tok_TripleGThan: return ">>>";
        case tok_Pound: return "#";
        case tok_Whitespace: return "WHITESPACE";
        case tok_Newline: return "NEWLINE";
        case tok_Begin: return "begin";
        case tok_Do: return "do";
        case tok_End: return "end";
        case tok_If: return "if";
        case tok_Else: return "else";
        case tok_Elif: return "elif";
        case tok_For: return "for";
        case tok_State: return "state";
        case tok_Def: return "def";
        case tok_Type: return "type";
        case tok_Return: return "return";
        case tok_In: return "in";
        case tok_True: return "true";
        case tok_False: return "false";
        case tok_Namespace: return "namespace";
        case tok_Include: return "include";
        case tok_And: return "and";
        case tok_Or: return "or";
        case tok_Not: return "not";
        case tok_Discard: return "discard";
        case tok_Null: return "null";
        case tok_Break: return "break";
        case tok_Continue: return "continue";
        case tok_Switch: return "switch";
        case tok_Case: return "case";
        case tok_While: return "while";
        case tok_Require: return "require";
        case tok_Package: return "package";
        case tok_Section: return "section";
        case tok_Unrecognized: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

std::string Token::toString() const
{
    std::stringstream out;
    out << get_token_text(match);
    return out.str();
}
int Token::length() const
{
    ca_assert(end >= start);
    return end - start;
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    int precedingIndent;
    std::vector<Token> *results;

    TokenizeContext(std::string const &_input, std::vector<Token> *_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(0),
          precedingIndent(-1),
          results(_results)
    { }

    char next(int lookahead=0) const
    {
        unsigned int index = nextIndex + lookahead;
        if (index >= input.length())
            return 0;
        return input[index];
    }

    char advanceChar()
    {
        if (finished())
            return 0;

        char c = next();
        this->nextIndex++;

        if (c == '\n') {
            this->linePosition++;
            this->charPosition = 0;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= (int) input.length();
    }

    bool withinRange(int lookahead) const {
        return nextIndex + lookahead < (int) input.length();
    }

    void consume(int match, int len) {

        Token instance;
        instance.match = match;
        instance.start = this->nextIndex;

        // Record where this token starts
        instance.lineStart = this->linePosition;
        instance.colStart = this->charPosition;

        for (int i=0; i < len; i++)
            advanceChar();

        // Record where this token ends
        instance.end = this->nextIndex;
        instance.lineEnd = this->linePosition;
        instance.colEnd = this->charPosition;

        // Update precedingIndent if this is the first whitespace on a line
        if (this->precedingIndent == -1) {
            if (instance.match == tok_Whitespace)
                this->precedingIndent = len;
            else
                this->precedingIndent = 0;
        }

        // NEWLINE token is a special case
        if (instance.match == tok_Newline) {
            instance.lineEnd = instance.lineStart;
            instance.colEnd = instance.colStart + 1;
            this->precedingIndent = -1;
        }

        instance.precedingIndent = this->precedingIndent;

        ca_assert(instance.lineStart >= 0);
        ca_assert(instance.lineEnd >= 0);
        ca_assert(instance.colStart >= 0);
        ca_assert(instance.colEnd >= 0);
        ca_assert(instance.lineStart <= instance.lineEnd);
        ca_assert((instance.colEnd > instance.colStart) ||
                instance.lineStart < instance.lineEnd);

        results->push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_name(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_comment(TokenizeContext& context);
void consume_multiline_comment(TokenizeContext& context);
bool match_number(TokenizeContext &context);
void consume_number(TokenizeContext &context);
void consume_hex_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);
void consume_triple_quoted_string_literal(TokenizeContext &context);
void consume_color_literal(TokenizeContext &context);

void tokenize(std::string const &input, TokenList* results)
{
    TokenizeContext context(input, results);

    while (!context.finished()) {
        top_level_consume_token(context);
    }
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_hexadecimal_digit(char c)
{
    return is_number(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_identifier_first_letter(char c)
{
    return is_letter(c) || c == '_';
}

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_' || c == ':';
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool is_newline(char c)
{
    return c == '\n';
}

bool try_to_consume_keyword(TokenizeContext& context, int keyword)
{
    const char* str = get_token_text(keyword);
    int str_len = (int) strlen(str);

    // Check if every letter matches
    for (int i=0; i < str_len; i++) {
        if (context.next(i) != str[i])
            return false;
    }

    // Check that this is really the end of the word
    if (is_acceptable_inside_identifier(context.next(str_len)))
        return false;

    // Don't match as a keyword if the next character is (. This might be
    // a bad idea.
    if (context.next(str_len) == '(')
        return false;

    // Keyword matches, now consume it
    context.consume(keyword, str_len);

    return true;
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_identifier_first_letter(context.next())) {

        if (context.next() <= 'm') {
            // a through m
            if (try_to_consume_keyword(context, tok_And)) return;
            if (try_to_consume_keyword(context, tok_Begin)) return;
            if (try_to_consume_keyword(context, tok_Break)) return;
            if (try_to_consume_keyword(context, tok_Case)) return;
            if (try_to_consume_keyword(context, tok_Continue)) return;
            if (try_to_consume_keyword(context, tok_Def)) return;
            if (try_to_consume_keyword(context, tok_Discard)) return;
            if (try_to_consume_keyword(context, tok_Do)) return;
            if (try_to_consume_keyword(context, tok_End)) return;
            if (try_to_consume_keyword(context, tok_Else)) return;
            if (try_to_consume_keyword(context, tok_Elif)) return;
            if (try_to_consume_keyword(context, tok_False)) return;
            if (try_to_consume_keyword(context, tok_For)) return;
            if (try_to_consume_keyword(context, tok_If)) return;
            if (try_to_consume_keyword(context, tok_In)) return;
            if (try_to_consume_keyword(context, tok_Include)) return;
        } else {
            // n through z
            if (try_to_consume_keyword(context, tok_Namespace)) return;
            if (try_to_consume_keyword(context, tok_Not)) return;
            if (try_to_consume_keyword(context, tok_Null)) return;
            if (try_to_consume_keyword(context, tok_Or)) return;
            if (try_to_consume_keyword(context, tok_Return)) return;
            if (try_to_consume_keyword(context, tok_State)) return;
            if (try_to_consume_keyword(context, tok_Switch)) return;
            if (try_to_consume_keyword(context, tok_True)) return;
            if (try_to_consume_keyword(context, tok_Type)) return;
            if (try_to_consume_keyword(context, tok_Require)) return;
            if (try_to_consume_keyword(context, tok_Package)) return;
            if (try_to_consume_keyword(context, tok_Section)) return;
            if (try_to_consume_keyword(context, tok_While)) return;
        }

        consume_identifier(context);
        return;
    }

    if (is_whitespace(context.next())) {
        consume_whitespace(context);
        return;
    }

    if (context.next() == '0'
        && context.next(1) == 'x') {
        consume_hex_number(context);
        return;
    }

    if (match_number(context)) {
        consume_number(context);
        return;
    }

    // Check for specific characters
    switch(context.next()) {
        case '(':
            context.consume(tok_LParen, 1);
            return;
        case ')':
            context.consume(tok_RParen, 1);
            return;
        case '{':
            if (context.next(1) == '-') {
                consume_multiline_comment(context);
                return;
            }
            context.consume(tok_LBrace, 1);
            return;
        case '}':
            context.consume(tok_RBrace, 1);
            return;
        case '[':
            context.consume(tok_LBracket, 1);
            return;
        case ']':
            context.consume(tok_RBracket, 1);
            return;
        case ',':
            context.consume(tok_Comma, 1);
            return;
        case '@':
            context.consume(tok_At, 1);
            return;
        case '=':
            if (context.next(1) == '=') {
                context.consume(tok_DoubleEquals, 2);
                return;
            } 

            context.consume(tok_Equals, 1);
            return;
        case '"':
        case '\'':
            consume_string_literal(context);
            return;
        case '\n':
            context.consume(tok_Newline, 1);
            return;
        case '.':
            if (context.next(1) == '.') {
                if (context.next(2) == '.') {
                    context.consume(tok_Ellipsis, 3); 
                } else {
                    context.consume(tok_TwoDots, 2);
                }
            } else if (context.next(1) == '@') {
                context.consume(tok_DotAt, 2);
            } else {
                context.consume(tok_Dot, 1);
            }
            return;
        case '?':
            context.consume(tok_Question, 1);
            return;
        case '*':
            if (context.next(1) == '=') {
                context.consume(tok_StarEquals, 2);
                return;
            }

            context.consume(tok_Star, 1);
            return;
        case '/':
            if (context.next(1) == '=') {
                context.consume(tok_SlashEquals, 2);
                return;
            }
            if (context.next(1) == '/') {
                context.consume(tok_DoubleSlash, 2);
                return;
            }
            context.consume(tok_Slash, 1);
            return;
        case '!':
            if (context.next(1) == '=') {
                context.consume(tok_NotEquals, 2);
                return;
            }
            break;

        case ':':
            if (context.next(1) == '=') {
                context.consume(tok_ColonEquals, 2);
                return;
            }
            else if (context.next(1) == ':') {
                context.consume(tok_DoubleColon, 2);
                return;
            } else if (is_identifier_first_letter(context.next(1))) {
                return consume_name(context);
            }

            context.consume(tok_Colon, 1);
            return;
        case '+':
            if (context.next(1) == '=') {
                context.consume(tok_PlusEquals, 2);
            } else {
                context.consume(tok_Plus, 1);
            }
            return;
        case '-':
            if (context.next(1) == '>') {
                context.consume(tok_RightArrow, 2);
                return;
            }

            if (context.next(1) == '-')
                return consume_comment(context);

            if (context.next(1) == '=') {
                context.consume(tok_MinusEquals, 2);
                return;
            }

            context.consume(tok_Minus, 1);
            return;

        case '<':
            if (context.next(1) == '<' && context.next(2) == '<') {
                consume_triple_quoted_string_literal(context);
                return;
            }

            if (context.next(1) == '=') {
                context.consume(tok_LThanEq, 2);
                return;
            }
            if (context.next(1) == '-') {
                context.consume(tok_LeftArrow, 2);
                return;
            }
            context.consume(tok_LThan, 1);
            return;

        case '>':
            if (context.next(1) == '=') {
                context.consume(tok_GThanEq, 2);
                return;
            }
            context.consume(tok_GThan, 1);
            return;

        case '%':
            context.consume(tok_Percent, 1);
            return;

        case '|':
            if (context.next(1) == '|') {
                context.consume(tok_DoubleVerticalBar, 2);
                return;
            }
            break;

        case '&':
            if (context.next(1) == '&') {
                context.consume(tok_DoubleAmpersand, 2);
                return;
            }

            context.consume(tok_Ampersand, 1);
            return;

        case ';':
            context.consume(tok_Semicolon, 1);
            return;

        case '#':
            consume_color_literal(context);
            return;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    context.consume(tok_Unrecognized, 1);
}

void consume_identifier(TokenizeContext &context)
{
    int lookahead = 0;
    while (is_acceptable_inside_identifier(context.next(lookahead)))
        lookahead++;

    context.consume(tok_Identifier, lookahead);
}

void consume_whitespace(TokenizeContext &context)
{
    int lookahead = 0;
    while (is_whitespace(context.next(lookahead)))
        lookahead++;

    context.consume(tok_Whitespace, lookahead);
}

void consume_comment(TokenizeContext& context)
{
    int lookahead = 0;
    while (context.withinRange(lookahead) && !is_newline(context.next(lookahead)))
        lookahead++;

    context.consume(tok_Comment, lookahead);
}

void consume_multiline_comment(TokenizeContext& context)
{
    int lookahead = 0;
    
    // Keep track of the current depth, for nested blocks.
    int depth = 0;

    while (context.withinRange(lookahead)) {
        if (context.next(lookahead) == '{' && context.next(lookahead + 1) == '-') {

            // Found a comment opener, increase depth. Also advance lookahead so that
            // we don't get confused by this: {-}
            lookahead += 2;
            depth++;
            continue;
        }

        if (context.next(lookahead) == '-' && context.next(lookahead + 1) == '}') {

            // Found a comment ender.
            depth--;
            lookahead += 2;

            if (depth == 0)
                break;

            continue;
        }

        lookahead++;
    }

    context.consume(tok_Comment, lookahead);
}

bool match_number(TokenizeContext &context)
{
    int lookahead = 0;

    if (context.next(lookahead) == '.')
        lookahead++;

    if (is_number(context.next(lookahead)))
        return true;

    return false;
}

void consume_number(TokenizeContext &context)
{
    int lookahead = 0;
    bool dot_encountered = false;

    // Possibly consume minus sign
    if (context.next(lookahead) == '-') {
        lookahead++;
    }

    while (true) {
        if (is_number(context.next(lookahead))) {
            lookahead++;
        } else if (context.next(lookahead) == '.') {
            // If we've already encountered a dot, finish and don't consume
            // this one.
            if (dot_encountered)
                break;

            // Special case: if this dot is followed by another dot, then it should
            // be tokenized as TWO_DOTS, so don't consume it here.
            if (context.next(lookahead+1) == '.')
                break;

            // Another special case, if the dot is followed by an identifier, then
            // don't consume it here. It might be an object call.
            if (is_identifier_first_letter(context.next(lookahead + 1)))
                break;

            // Otherwise, consume the dot
            lookahead++;
            dot_encountered = true;
        }
        else {
            break;
        }
    }

    if (dot_encountered)
        context.consume(tok_Float, lookahead);
    else
        context.consume(tok_Integer, lookahead);
}

void consume_hex_number(TokenizeContext &context)
{
    int lookahead = 0;

    // consume the 0x part
    lookahead += 2;

    while (is_hexadecimal_digit(context.next(lookahead)))
        lookahead++;

    context.consume(tok_HexInteger, lookahead);
}

void consume_string_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // Consume starting quote, this can be ' or "
    char quote_type = context.next();
    lookahead++;

    bool escapedNext = false;
    while (context.withinRange(lookahead)) {

        char c = context.next(lookahead);

        if (c == quote_type && !escapedNext)
            break;

        if (c == '\\' && !escapedNext)
            escapedNext = true;
        else
            escapedNext = false;

        lookahead++;
    }

    // consume ending quote
    lookahead++;

    context.consume(tok_String, lookahead);
}

void consume_triple_quoted_string_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // Consume initial <<<
    lookahead += 3;

    while (context.withinRange(lookahead) &&
            !(context.next(lookahead) == '>'
                && context.next(lookahead + 1) == '>'
                && context.next(lookahead + 2) == '>'))
        lookahead++;

    // Consume closing >>>
    lookahead += 3;
    context.consume(tok_String, lookahead);
}

void consume_color_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // consume #
    lookahead++;

    while (is_hexadecimal_digit(context.next(lookahead)))
        lookahead++;

    int hex_digits = lookahead - 1;

    // acceptable lengths are 3, 4, 6 or 8 characters (not including #)
    if (hex_digits == 3 || hex_digits == 4 || hex_digits == 6 || hex_digits == 8)
        context.consume(tok_Color, lookahead);
    else
        context.consume(tok_Unrecognized, lookahead);
}

void consume_name(TokenizeContext &context)
{
    int lookahead = 0;

    // consume the leading :
    lookahead++;

    while (is_acceptable_inside_identifier(context.next(lookahead)))
        lookahead++;

    context.consume(tok_Name, lookahead);
}

void TokenStream::reset(caValue* inputString)
{
    reset(as_string(inputString));
}

Token const&
TokenStream::next(int lookahead) const
{
    int i = this->_position + lookahead;

    if (i >= (int) tokens.size())
        throw std::runtime_error("index out of bounds");

    if (i < 0)
        throw std::runtime_error("index < 0");

    return tokens[i];
}

std::string TokenStream::nextStr(int lookahead) const
{
    int startPos = next(lookahead).start;
    int length = next(lookahead).length();

    ca_assert(length > 0);
    return std::string(_sourceText.c_str() + startPos, length);
}

void TokenStream::getNextStr(caValue* value, int lookahead) const
{
    int startPos = next(lookahead).start;
    int length = next(lookahead).length();
    circa_set_string_size(value, _sourceText.c_str() + startPos, length);
}

bool TokenStream::nextIsEof(int lookahead) const
{
    int i = this->_position + lookahead;
    return i >= (int) tokens.size();
}

bool TokenStream::nextIs(int match, int lookahead) const
{
    if ((this->_position + lookahead) >= tokens.size())
        return false;
    if ((int(this->_position) + lookahead) < 0)
        return false;
        
    return next(lookahead).match == match;
}

void
TokenStream::consume(int match)
{
    if (finished())
        throw std::runtime_error(std::string("Unexpected EOF while looking for ")
                + get_token_text(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << get_token_text(match)
            << ", found " << get_token_text(next().match)
            << " '" << nextStr() << "')";
        throw std::runtime_error(msg.str());
    }

    _position++;
}

std::string
TokenStream::consumeStr(int match)
{
    std::string out = nextStr();
    consume(match);
    return out;
}

void
TokenStream::consumeStr(caValue* output, int match)
{
    getNextStr(output, 0);
    consume(match);
}

Name
TokenStream::consumeName(int match)
{
    Name value = name_from_string(nextStr().c_str());
    consume(match);
    return value;
}

int
TokenStream::getPosition() const
{
    return _position;
}

void
TokenStream::resetPosition(int loc)
{
    ca_assert(loc >= 0);
    _position = loc;
}

std::string
TokenStream::toString() const
{
    std::stringstream out;

    out << "{index: " << _position << ", ";
    out << "tokens: [";

    bool first = true;

    for (unsigned int i=0; i < tokens.size(); i++) {
        if (!first) out << ", ";
        out << tokens[i].toString();
        first = false;
    }
    out << "]}";
    return out.str();
}

void TokenStream::dump()
{
    int lookbehind = 5;
    int lookahead = 15;
    for (int i=-lookbehind; i < lookahead; i++) {
        int index = position() + i;
        if (index < 0) continue;
        if (index >= length()) continue;

        std::cout << "[" << i << "] " << get_token_text(next(i).match)
            << " '" << nextStr(i) << "'" << std::endl;
    }
}

void print_remaining_tokens(std::ostream& out, TokenStream& tokens)
{
    for (int i=0; i < tokens.remaining(); i++) {
        if (i != 0) out << " ";
        out << get_token_text(tokens.next(i).match);
        out << "(" << tokens.nextStr(i) << ")";
    }
}

} // namespace circa
