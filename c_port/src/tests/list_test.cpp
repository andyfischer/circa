
#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace list_test {

void test_range()
{
    Branch* branch = new Branch();

    Term* range_zero_to_ten = quick_exec_function(branch, "range(10)");

    test_assert(as_int(as_list(range_zero_to_ten)->get(0)) == 0);
    test_assert(as_int(as_list(range_zero_to_ten)->get(9)) == 9);
}

void test_list_apply()
{
    Branch* branch = new Branch();

    quick_exec_function(branch, "list-apply(print, list-apply(to-string, range(5)))");
}

void list_test()
{
    test_range();
    test_list_apply();
}

}}
