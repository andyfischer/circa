// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace introspection_tests {

void test_is_value()
{
    Branch branch;

    test_assert(is_value(create_value(&branch, &INT_T)));
    test_assert(is_value(create_value(&branch, &STRING_T)));
    test_assert(is_value(create_value(&branch, &BOOL_T)));
    test_assert(!is_value(branch.compile("1 + 2")));
}

void test_get_involved_terms()
{
    Branch branch;

    Term* a = branch.compile("a = 1.0");
    branch.compile("b = 2.0");
    Term* c = branch.compile("c = add(a,b)");
    Term* d = branch.compile("d = add(c,5.0)");
    branch.compile("e = add(c,5.0)");

    TermList subtree = get_involved_terms(TermList(a), TermList(d));

    test_equals(subtree, TermList(a,c,d));
}

void test_term_to_raw_string()
{
    // Test that term_to_raw_string doesn't die if a term has a NULL function or type.
    Term* term = alloc_term();

    get_term_to_string_extended(term);

    delete term;
}

bool name_visitor_that_appends_to_list(Term* term, const char* name, TaggedValue* list)
{
    set_ref(List::checkCast(list)->append(), term);
    return false;
}

void test_visit_name_accessible_terms()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 1");
    Branch* ns = create_namespace(&branch, "ns");
    Term* c = ns->compile("c = 1");
    Term* d = ns->compile("d = 1");
    /*Term* e =*/ ns->compile("e = 1");
    branch.compile("f = 1");

    TaggedValue results;
    set_list(&results);
    visit_name_accessible_terms(d, name_visitor_that_appends_to_list, &results);

    List* list = List::checkCast(&results);
    test_assert(list->length() == 3);
    test_assert(as_ref(list->get(0)) == c);
    test_assert(as_ref(list->get(1)) == b);
    test_assert(as_ref(list->get(2)) == a);
}

void register_tests()
{
    REGISTER_TEST_CASE(introspection_tests::test_is_value);
    REGISTER_TEST_CASE(introspection_tests::test_get_involved_terms);
    REGISTER_TEST_CASE(introspection_tests::test_term_to_raw_string);
    REGISTER_TEST_CASE(introspection_tests::test_visit_name_accessible_terms);
}

} // namespace introspection_tests

} // namespace circa
