// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ref_set.h"
#include "dictionary.h"
#include "testing.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {
namespace container_tests {

/*
void test_reference()
{
    Term* term1 = new Term();
    Term* term2 = new Term();

    Reference ref1(term1);

    test_assert(ref1._term == term1);
    test_assert(term1->references == 1);

    Reference ref2;
    ref2 = term1;

    test_assert(ref2._term == term1);
    test_assert(term1->references == 2);

    ref1 = term2;

    test_assert(term1->references == 1);
    test_assert(term2->references == 1);

    ref2 = term2;
    test_assert(term1->references == 0);
    test_assert(term2->references == 2);

    {
        Reference tempref(term2);
        test_assert(tempref._term == term2);
        test_assert(term2->references == 3);
    }

    test_assert(term2->references == 2);

    Reference ref3(ref1);
    test_assert(ref3._term == term2);
    test_assert(term2->references == 3);

    ref1 = term1;
    ref2 = ref1;

    test_assert(ref2._term == term1);
    test_assert(term1->references == 2);

    term1->name = "hi";
    test_assert((*ref1).name == "hi");
    test_assert(ref1->name == "hi");
}
*/

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
}

void test_dictionary()
{
    Dictionary dict;

    test_assert(!dict.contains("a"));

    dict.addSlot("a", INT_TYPE);
    as_int(dict["a"]) = 4;
    dict.addSlot("b", INT_TYPE);
    as_int(dict["b"]) = 6;

    test_assert(dict.contains("a"));
    test_assert(dict.contains("b"));

    test_assert(as_int(dict["a"]) == 4);
    test_assert(as_int(dict["b"]) == 6);

    //dict.clear(); <-- this causes us to crash later :(
    /*

    test_assert(!dict.contains("a"));
    test_assert(!dict.contains("b"));
    */
}

} // namespace container_tests

void register_container_tests()
{
    // REGISTER_TEST_CASE(container_tests::test_reference);
    REGISTER_TEST_CASE(container_tests::test_set);
    REGISTER_TEST_CASE(container_tests::test_namespace);
    REGISTER_TEST_CASE(container_tests::test_list);
    REGISTER_TEST_CASE(container_tests::test_dictionary);
}

} // namespace circa
