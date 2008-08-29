
#include "common_headers.h"

#include "tests/common.h"
#include "term.h"
#include "term_set.h"
#include "term_list.h"

namespace circa {
namespace container_tests {

void test_set()
{
    TermSet set;

    Term* term1 = new Term();

    test_assert(!set.contains(term1));
    test_assert(set.count() == 0);

    set.add(term1);

    test_assert(set.contains(term1));
    test_assert(set.count() == 1);

    set.add(term1);

    test_assert(set.contains(term1));
    test_assert(set.count() == 1);

    Term* term2 = new Term();
    set.add(term2);

    test_assert(set.contains(term2));
    test_assert(set.count() == 2);

    set.remove(term1);

    test_assert(!set.contains(term1));
    test_assert(set.count() == 1);

    set.remove(term2);

    test_assert(!set.contains(term2));
    test_assert(set.count() == 0);
}

} // namespace container_tests

void register_container_tests()
{
    REGISTER_TEST_CASE(container_tests::test_set);
}

} // namespace circa
