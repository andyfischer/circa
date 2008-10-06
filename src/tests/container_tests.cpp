// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace container_tests {

void test_set()
{
    ReferenceSet set;

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

    delete term1;
    delete term2;
}

void test_namespace()
{
    TermNamespace nspace;
    Term *term = new Term();

    nspace.bind(term, "a");
    test_assert(nspace.contains("a"));
    test_assert(nspace["a"] == term);

    Term *term2 = new Term();
    ReferenceMap remap;
    remap[term] = term2;
    nspace.remapPointers(remap);
    test_assert(nspace["a"] == term2);

    test_assert(nspace.contains("a"));
    remap[term2] = NULL;
    nspace.remapPointers(remap);
    test_assert(!nspace.contains("a"));

    delete term;
    delete term2;
}

void test_list()
{
    ReferenceList list;
    Term* term = new Term();
    Term* term2 = new Term();

    test_assert(list.count() == 0);

    list.append(term);
    list.append(term2);
    test_assert(list.count() == 2);
    test_assert(list[0] == term);
    test_assert(list[1] == term2);

    Term* term3 = new Term();
    ReferenceMap remap;
    remap[term] = term3;
    list.remapPointers(remap);
    test_assert(list.count() == 2);
    test_assert(list[0] == term3);
    test_assert(list[1] == term2);

    list.clear();

    test_assert(list.count() == 0);

    delete term;
    delete term2;
    delete term3;
}

} // namespace container_tests

void register_container_tests()
{
    REGISTER_TEST_CASE(container_tests::test_set);
    REGISTER_TEST_CASE(container_tests::test_namespace);
    REGISTER_TEST_CASE(container_tests::test_list);
}

} // namespace circa
