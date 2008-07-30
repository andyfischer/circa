
#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace list_test {

void test_range()
{
    Branch* branch = new Branch();

    Term* range_zero_to_ten = quick_exec_function(branch, "range(10)");

}

void list_test()
{
    test_range();
}

}}
