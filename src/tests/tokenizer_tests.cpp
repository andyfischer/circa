// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "tests/common.h"
#include "branch.h"
#include "parser.h"
#include "builtins.h"
#include "term.h"
#include "runtime.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "errors.h"

namespace token = circa::tokenizer;

namespace circa {
namespace tokenizer_tests {

void test_identifiers()
{
    token::TokenList results;
    token::tokenize("word has_underscore has-hyphen,hasnumbers183", results);

    test_assert(results.size() == 7);

    test_assert(results[0].text == "word");
    test_assert(results[0].match == token::IDENTIFIER);
    test_assert(results[1].text == " ");
    test_assert(results[1].match == token::WHITESPACE);
    test_assert(results[2].text == "has_underscore");
    test_assert(results[2].match == token::IDENTIFIER);
    test_assert(results[3].text == " ");
    test_assert(results[3].match == token::WHITESPACE);
    test_assert(results[4].text == "has-hyphen");
    test_assert(results[4].match == token::IDENTIFIER);
    test_assert(results[5].text == ",");
    test_assert(results[5].match == token::COMMA);
    test_assert(results[6].text == "hasnumbers183");
    test_assert(results[6].match == token::IDENTIFIER);
}

void test_integers()
{
    token::TokenList results;
    token::tokenize("1 0 1234567890 -3", results);

    test_assert(results.size() == 7);
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
}

void test_floats()
{
    token::TokenList results;
    token::tokenize("1.0 16. .483 .123. -0.1 -.54", results);

    test_assert(results[0].text == "1.0");
    test_assert(results[0].match == token::FLOAT);
    test_assert(results[2].text == "16.");
    test_assert(results[2].match == token::FLOAT);
    test_assert(results[4].text == ".483");
    test_assert(results[4].match == token::FLOAT);
    test_assert(results[6].text == ".123");
    test_assert(results[6].match == token::FLOAT);
    test_assert(results[7].text == ".");
    test_assert(results[7].match == token::DOT);
    test_assert(results[9].text == "-0.1");
    test_assert(results[9].match == token::FLOAT);
    test_assert(results[11].text == "-.54");
    test_assert(results[11].match == token::FLOAT);
    test_assert(results.size() == 12);
}

void test_symbols()
{
    token::TokenList results;
    token::tokenize(",()=?][", results);

    test_assert(results.size() == 7);
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
}

void test_string_literal()
{
    token::TokenList results;
    token::tokenize("\"string literal\"", results);

    test_assert(results.size() == 1);
    test_assert(results[0].text == "string literal");
    test_assert(results[0].match == token::STRING);
}

void test_token_stream()
{
    token_stream::TokenStream tstream("1 2.0");

    test_assert(tstream.nextIs(tokenizer::INTEGER));
    test_assert(tstream.nextIs(tokenizer::WHITESPACE, 1));
    test_assert(tstream.nextNonWhitespaceIs(tokenizer::FLOAT, 1));
}

void token_stream_to_string()
{
    token_stream::TokenStream tstream("hi + 0.123");

    test_assert(tstream.toString() ==
            "{index: 0, tokens: [IDENTIFIER \"hi\", WHITESPACE \" \", + \"+\", "
            "WHITESPACE \" \", FLOAT \"0.123\"]}");
}

void hosted_tokenize()
{
    Branch branch;
    Term* result = eval_statement(branch, "\"hi + 0.123\" -> tokenize -> to-string");

    test_assert(as_string(result) == 
            "{index: 0, tokens: [IDENTIFIER \"hi\", WHITESPACE \" \", + \"+\", "
            "WHITESPACE \" \", FLOAT \"0.123\"]}");
}

} // namespace tokenizer_tests

void register_tokenizer_tests()
{
    REGISTER_TEST_CASE(tokenizer_tests::test_identifiers);
    REGISTER_TEST_CASE(tokenizer_tests::test_integers);
    REGISTER_TEST_CASE(tokenizer_tests::test_floats);
    REGISTER_TEST_CASE(tokenizer_tests::test_symbols);
    REGISTER_TEST_CASE(tokenizer_tests::test_string_literal);
    REGISTER_TEST_CASE(tokenizer_tests::test_token_stream);
    REGISTER_TEST_CASE(tokenizer_tests::token_stream_to_string);
    REGISTER_TEST_CASE(tokenizer_tests::hosted_tokenize);
}

} // namespace circa
