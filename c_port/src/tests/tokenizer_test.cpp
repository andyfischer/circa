
#include "common_headers.h"

#include "errors.h"
#include "parser/token.h"

class TestFailure : public errors::CircaError
{
    virtual std::string message()
    {
        return "Test failure";
    }
};

void test_assert_f(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line: " << line;
        throw errors::CircaError(msg.str());
    }
}

#define test_assert(c) test_assert_f((c), __LINE__, __FILE__)


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
    token::tokenize("1 0 1234567890", results);

    test_assert(results.size() == 5);
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
}

void test_symbols()
{
    token::TokenList results;
    token::tokenize(",()", results);

    test_assert(results.size() == 3);
    test_assert(results[0].text == ",");
    test_assert(results[0].match == token::COMMA);
    test_assert(results[1].text == "(");
    test_assert(results[1].match == token::LPAREN);
    test_assert(results[2].text == ")");
    test_assert(results[2].match == token::RPAREN);
}

void tokenize_test()
{
    test_identifiers();
    test_integers();
    test_symbols();
}
