
#include "common_headers.h"

#include "tests/common.h"
#include "term.h"
#include "term_list.h"
#include "term_namespace.h"
#include "term_set.h"

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

    set.add(term1);
    test_assert(set.contains(term1));
    test_assert(set.count() == 1);

    /*TermMap remap;
    remap[term1] = term2;
    set.remapPointers(remap);
    test_assert(set.contains(term2));
    test_assert(set.count() == 1);*/
}

void test_namespace()
{
    TermNamespace nspace;
    Term *term = new Term();

    nspace.bind(term, "a");
    test_assert(nspace.contains("a"));
    test_assert(nspace["a"] == term);

    Term *term2 = new Term();
    TermMap remap;
    remap[term] = term2;
    nspace.remapPointers(remap);
    test_assert(nspace["a"] == term2);

    test_assert(nspace.contains("a"));
    remap[term2] = NULL;
    nspace.remapPointers(remap);
    test_assert(!nspace.contains("a"));
}

} // namespace container_tests

void register_container_tests()
{
    REGISTER_TEST_CASE(container_tests::test_set);
    REGISTER_TEST_CASE(container_tests::test_namespace);
}

} // namespace circa
