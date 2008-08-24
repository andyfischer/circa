
#include "common.h"

namespace circa {

std::vector<TestCase> gTestCases;

void test_assert_f(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line: " << line;
        throw errors::CircaError(msg.str());
    }
}

} // namespace circa
