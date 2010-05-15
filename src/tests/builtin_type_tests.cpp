// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace builtin_type_tests {

void test_reference()
{
    Branch branch;

    Term* r1 = create_value(branch, REF_TYPE);
    Term* a = create_value(branch, INT_TYPE);
    Term* r2 = create_value(branch, REF_TYPE);

    as_ref(r1) = a;

    test_assert(as_ref(r1) == a);

    copy(r1, r2);

    test_assert(as_ref(r2) == a);
}

void reference_type_deletion_bug()
{
    // There used to be a bug where deleting a reference term would delete
    // the thing it was pointed to.
    Branch *branch = new Branch();

    Term* myref = create_value(*branch, REF_TYPE);

    myref->asRef() = INT_TYPE;

    delete branch;

    assert_valid_term(INT_TYPE);
    test_assert(INT_TYPE->type != NULL);
}

void test_set()
{
    Branch branch;

    Term* s = branch.eval("s = Set()");

    test_assert(s->numElements() == 0);

    s = branch.eval("s.add(1)");

    test_assert(s->numElements() == 1);
    test_assert(s->getIndex(0)->asInt() == 1);

    s = branch.eval("s.add(1)");
    test_assert(s->numElements() == 1);

    s = branch.eval("s.add(2)");
    test_assert(branch);
    test_assert(s->numElements() == 2);

    s = branch.eval("s.remove(1)");
    test_assert(s->numElements() == 1);
    test_assert(s->getIndex(0)->asInt() == 2);

    // check that things are copied by value
    Term* val = branch.eval("val = 5");
    s = branch.eval("s.add(val)");

    test_assert(s->getIndex(1)->asInt() == 5);
    set_int(val, 6);
    test_assert(s->getIndex(1)->asInt() == 5);
}

void test_list()
{
    Branch branch;

    Term* l = branch.eval("l = List()");

#ifdef NEWLIST
    test_assert(list_t::is_list(l));
#endif
    test_assert(l->numElements() == 0);

    l = branch.eval("l.append(2)");
    test_assert(l->numElements() == 1);
    test_assert(l->getIndex(0)->asInt() == 2);
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

void test_list2()
{
    RefList list;
    Term* term = new Term();
    Term* term2 = new Term();

    test_assert(list.length() == 0);

    list.append(term);
    list.append(term2);
    test_assert(list.length() == 2);
    test_assert(list[0] == term);
    test_assert(list[1] == term2);

    Term* term3 = new Term();
    ReferenceMap remap;
    remap[term] = term3;
    list.remapPointers(remap);
    test_assert(list.length() == 2);
    test_assert(list[0] == term3);
    test_assert(list[1] == term2);

    list.clear();

    test_assert(list.length() == 0);
}


void register_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_reference);
    REGISTER_TEST_CASE(builtin_type_tests::reference_type_deletion_bug);
    REGISTER_TEST_CASE(builtin_type_tests::test_set);
    REGISTER_TEST_CASE(builtin_type_tests::test_list);
    REGISTER_TEST_CASE(builtin_type_tests::test_namespace);
    REGISTER_TEST_CASE(builtin_type_tests::test_list2);
}

} // namespace builtin_type_tests

} // namespace circa
