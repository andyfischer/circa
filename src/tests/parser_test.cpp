
#include "common_headers.h"

#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace parser_test {

void test_atoms()
{
    token_stream::TokenStream tokens("1.0");
    ast::Expression *expr = parser::atom(tokens);
    test_assert(dynamic_cast<ast::LiteralFloat*>(expr) != NULL);
}

void all_tests()
{
    test_atoms();
}

} // namespace parser_test
} // namespace circa
