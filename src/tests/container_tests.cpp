// Copyright 2008 Andrew Fischer

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

    /*TermMap remap;
    remap[term1] = term2;
    set.remapPointers(remap);
    test_assert(set.contains(term2));
    test_assert(set.count() == 1);*/

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

void value_map()
{
    Branch branch;
    ValueMap map;

    Term* two = constant_int(&branch, 2);
    Term* another_two = constant_int(&branch, 2);
    Term* hi = constant_string(&branch, "hi");
    Term* bye = constant_string(&branch, "bye");

    map.set(two, hi);

    // Change our version of hi to verify that ValueMap has made a duplicate
    as_string(hi) = "hello";

    test_assert(as_string(map.findValueForKey(another_two)) == "hi");

    map.set(two, hi);

    test_assert(as_string(map.findValueForKey(another_two)) == "hello");

    map.set(two, bye);

    test_assert(as_string(map.findValueForKey(another_two)) == "bye");
}

void value_map_from_source()
{
    Branch branch;

    eval_statement(branch, "myMap = Map()");
    eval_statement(branch, "myMap = map-set(myMap, 'a, 2)");
    eval_statement(branch, "myMap = map-set(myMap, 'b, 5)");

    Term* a = eval_statement(branch, "map-access(myMap, 'a)");
    test_assert(as_int(a) == 2);

    Term* b = eval_statement(branch, "myMap('b)");
    test_assert(as_int(b) == 5);
}

void value_list_source()
{
    Branch branch;

    //todo
}

} // namespace container_tests

void register_container_tests()
{
    REGISTER_TEST_CASE(container_tests::test_set);
    REGISTER_TEST_CASE(container_tests::test_namespace);
    REGISTER_TEST_CASE(container_tests::test_list);
    REGISTER_TEST_CASE(container_tests::value_map);
    REGISTER_TEST_CASE(container_tests::value_map_from_source);
}

} // namespace circa
