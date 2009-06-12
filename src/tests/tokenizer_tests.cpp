// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace token = circa::tokenizer;

namespace circa {
namespace tokenizer_tests {

void test_identifiers()
{
    token::TokenList results;
    token::tokenize("word has_underscore has_hyphen,hasnumbers183", results);

    test_assert(results.size() == 7);

    test_assert(results[0].text == "word");
    test_assert(results[0].match == token::IDENTIFIER);
    test_assert(results[1].text == " ");
    test_assert(results[1].match == token::WHITESPACE);
    test_assert(results[2].text == "has_underscore");
    test_assert(results[2].match == token::IDENTIFIER);
    test_assert(results[3].text == " ");
    test_assert(results[3].match == token::WHITESPACE);
    test_assert(results[4].text == "has_hyphen");
    test_assert(results[4].match == token::IDENTIFIER);
    test_assert(results[5].text == ",");
    test_assert(results[5].match == token::COMMA);
    test_assert(results[6].text == "hasnumbers183");
    test_assert(results[6].match == token::IDENTIFIER);
}

void test_integers()
{
    token::TokenList results;
    token::tokenize("1 0 1234567890 -3 0x123", results);

    test_assert(results.size() == 9);
    test_assert(results[0].text == "1");
    test_assert(results[0].match == token::INTEGER);
    test_assert(results[1].text == " ");
    test_assert(results[1].match == token::WHITESPACE);
    test_assert(results[2].text == "0");
    test_assert(results[2].match == token::INTEGER);
    test_assert(results[3].text == " ");
    test_assert(results[3].match == token::WHITESPACE);
    test_assert(results[4].text == "1234567890");
    test_assert(results[4].match == token::INTEGER);
    test_assert(results[5].text == " ");
    test_assert(results[5].match == token::WHITESPACE);
    test_assert(results[6].text == "-3");
    test_assert(results[6].match == token::INTEGER);
    test_assert(results[8].text == "0x123");
    test_assert(results[8].match == token::HEX_INTEGER);
}

void test_floats()
{
    token::TokenList results;
    token::tokenize("1.0 16. .483 .123. -0.1 -.54", results);

    test_assert(results.size() == 12);
    test_assert(results[0].text == "1.0");
    test_assert(results[0].match == token::FLOAT_TOKEN);
    test_assert(results[2].text == "16.");
    test_assert(results[2].match == token::FLOAT_TOKEN);
    test_assert(results[4].text == ".483");
    test_assert(results[4].match == token::FLOAT_TOKEN);
    test_assert(results[6].text == ".123");
    test_assert(results[6].match == token::FLOAT_TOKEN);
    test_assert(results[7].text == ".");
    test_assert(results[7].match == token::DOT);
    test_assert(results[9].text == "-0.1");
    test_assert(results[9].match == token::FLOAT_TOKEN);
    test_assert(results[11].text == "-.54");
    test_assert(results[11].match == token::FLOAT_TOKEN);

    token::TokenList results2;
    token::tokenize("5.200", results2);

    test_assert(results2.size() == 1);
    test_assert(results2[0].text == "5.200");
    test_assert(results2[0].match == token::FLOAT_TOKEN);
}

void test_symbols1()
{
    token::TokenList results;
    token::tokenize(",()=?][<=>=", results);

    test_assert(results.size() == 9);
    test_assert(results[0].text == ",");
    test_assert(results[0].match == token::COMMA);
    test_assert(results[1].text == "(");
    test_assert(results[1].match == token::LPAREN);
    test_assert(results[2].text == ")");
    test_assert(results[2].match == token::RPAREN);
    test_assert(results[3].text == "=");
    test_assert(results[3].match == token::EQUALS);
    test_assert(results[4].text == "?");
    test_assert(results[4].match == token::QUESTION);
    test_assert(results[5].text == "]");
    test_assert(results[5].match == token::RBRACKET);
    test_assert(results[6].text == "[");
    test_assert(results[6].match == token::LBRACKET);
    test_assert(results[7].text == "<=");
    test_assert(results[7].match == token::LTHANEQ);
    test_assert(results[8].text == ">=");
    test_assert(results[8].match == token::GTHANEQ);
}

void test_symbols2()
{
    token::TokenList results;
    token::tokenize("<>:;%...<-//", results);
    test_assert(results.size() == 8);
    test_assert(results[0].text == "<");
    test_assert(results[0].match == token::LTHAN);
    test_assert(results[1].text == ">");
    test_assert(results[1].match == token::GTHAN);
    test_assert(results[2].text == ":");
    test_assert(results[2].match == token::COLON);
    test_assert(results[3].text == ";");
    test_assert(results[3].match == token::SEMICOLON);
    test_assert(results[4].text == "%");
    test_assert(results[4].match == token::PERCENT);
    test_assert(results[5].text == "...");
    test_assert(results[5].match == token::ELLIPSIS);
    test_assert(results[6].text == "<-");
    test_assert(results[6].match == token::LEFT_ARROW);
    test_assert(results[7].text == "//");
    test_assert(results[7].match == token::DOUBLE_SLASH);
}

void test_keywords()
{
    token::TokenList results;
    token::tokenize("end,if,else,for,state", results);

    test_assert(results.size() == 9);
    test_assert(results[0].text == "end");
    test_assert(results[0].match == token::END);
    test_assert(results[2].text == "if");
    test_assert(results[2].match == token::IF);
    test_assert(results[4].text == "else");
    test_assert(results[4].match == token::ELSE);
    test_assert(results[6].text == "for");
    test_assert(results[6].match == token::FOR);
    test_assert(results[8].text == "state");
    test_assert(results[8].match == token::STATE);
}

void test_identifiers_that_look_like_keywords()
{
    token::TokenList results;
    token::tokenize("endup,iffy,else_,stateful", results);

    test_assert(results.size() == 7);
    test_assert(results[0].text == "endup");
    test_assert(results[0].match == token::IDENTIFIER);
    test_assert(results[2].text == "iffy");
    test_assert(results[2].match == token::IDENTIFIER);
    test_assert(results[4].text == "else_");
    test_assert(results[4].match == token::IDENTIFIER);
    test_assert(results[6].text == "stateful");
    test_assert(results[6].match == token::IDENTIFIER);
}

void test_string_literal()
{
    token::TokenList results;
    token::tokenize("\"string literal\"", results);

    test_assert(results.size() == 1);
    test_assert(results[0].text == "\"string literal\"");
    test_assert(results[0].match == token::STRING);
}

void test_token_stream()
{
    TokenStream tstream("1 2.0");

    test_assert(tstream.nextIs(tokenizer::INTEGER));
    test_assert(tstream.nextIs(tokenizer::WHITESPACE, 1));
    test_assert(tstream.nextNonWhitespaceIs(tokenizer::FLOAT_TOKEN, 1));
}

void token_stream_to_string()
{
    TokenStream tstream("hi + 0.123");

    test_assert(tstream.toString() ==
            "{index: 0, tokens: [IDENTIFIER \"hi\", WHITESPACE \" \", + \"+\", "
            "WHITESPACE \" \", FLOAT \"0.123\"]}");
}

bool token_location_equals(token::Token& inst, int colStart, int colEnd, int lineStart, int lineEnd)
{
    return (inst.colStart == colStart) && (inst.colEnd == colEnd) && (inst.lineStart == lineStart) && (inst.lineEnd == lineEnd);
}

void test_locations()
{
    token::TokenList results;
    token::tokenize("hello 1234", results);

    test_assert(results.size() == 3);
    test_assert(token_location_equals(results[0], 0, 5, 1, 1));
    test_assert(token_location_equals(results[1], 5, 6, 1, 1));
    test_assert(token_location_equals(results[2], 6, 10, 1, 1));

    // Now try with some newlines
    results.clear();
    token::tokenize("hey  \n87654+\n\n1", results);
    test_assert(results.size() == 8);
    test_assert(token_location_equals(results[0], 0, 3, 1, 1));
    test_assert(token_location_equals(results[1], 3, 5, 1, 1));
    test_assert(token_location_equals(results[2], 5, 6, 1, 1)); // newline
    test_assert(token_location_equals(results[3], 0, 5, 2, 2));
    test_assert(token_location_equals(results[4], 5, 6, 2, 2));
    test_assert(token_location_equals(results[5], 6, 7, 2, 2)); // newline
    test_assert(token_location_equals(results[6], 0, 1, 3, 3)); // newline
    test_assert(token_location_equals(results[7], 0, 1, 4, 4));
}

void test_consume_line()
{
    TokenStream tokens("for in $#!$#@ 151 poop \nfin");

    // happily consume 'for' and some whitespace
    test_assert(tokens.nextIs(token::FOR));
    tokens.consumet();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consumet();

    int startPosition = tokens.getPosition();

    // happily consume some more stuff
    test_assert(tokens.nextIs(token::IN_TOKEN));
    tokens.consumet();
    test_assert(tokens.nextIs(token::WHITESPACE));
    tokens.consumet();

    // now freak out
    std::string errorline = consume_line(tokens, startPosition);

    test_equals(errorline, "in $#!$#@ 151 poop ");
    test_assert(tokens.nextIs(token::IDENTIFIER));
    test_assert(tokens.next().text == "fin");
}

void register_tests()
{
    REGISTER_TEST_CASE(tokenizer_tests::test_identifiers);
    REGISTER_TEST_CASE(tokenizer_tests::test_integers);
    REGISTER_TEST_CASE(tokenizer_tests::test_floats);
    REGISTER_TEST_CASE(tokenizer_tests::test_symbols1);
    REGISTER_TEST_CASE(tokenizer_tests::test_symbols2);
    REGISTER_TEST_CASE(tokenizer_tests::test_keywords);
    REGISTER_TEST_CASE(tokenizer_tests::test_identifiers_that_look_like_keywords);
    REGISTER_TEST_CASE(tokenizer_tests::test_string_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_token_stream);
    REGISTER_TEST_CASE(tokenizer_tests::token_stream_to_string);
    REGISTER_TEST_CASE(tokenizer_tests::test_locations);
    REGISTER_TEST_CASE(tokenizer_tests::test_consume_line);
}

} // namespace tokenizer_tests

} // namespace circa
