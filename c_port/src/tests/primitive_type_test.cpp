
#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace primitive_type_test {

void test_strings()
{
    Branch* branch = new Branch();
    Term str1 = constant_string(branch, "one");
    Term str2 = constant_string(branch, "two");

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "two");
    
    copy_term(str1,str2);

    test_assert(as_string(str1) == "one");
    test_assert(as_string(str2) == "one");
}

void primitive_type_test()
{
    test_strings();
}

}}
