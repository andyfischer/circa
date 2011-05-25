// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "types/handle.h"

namespace circa {
namespace gc_tests {

void test_visit_heap()
{
    List list1;
    set_string(list1.append(), "element0");
    set_string(list1.append(), "element1");
    List& list2 = *List::cast(list1.append(), 0);
    set_string(list2.append(), "element3");
    Dict& dict1 = *Dict::cast(list1.append());
    set_string(dict1.insert("a"), "element4");
    set_string(dict1.insert("b"), "element5");

    //recursive_dump_heap(&list1, "list1");
}

void test_count_references()
{
    int myobject;
    TaggedValue handle;
    handle_t::set(&handle, &HANDLE_T, &myobject);

    test_assert(count_references_to_pointer(&handle, &myobject) == 1);

    List list;
    list.resize(3);
    copy(&handle, list[0]);
    copy(&handle, list[1]);
    copy(&handle, list[2]);

    test_assert(count_references_to_pointer(&list, &myobject) == 3);
    set_null(&list);
    test_assert(count_references_to_pointer(&list, &myobject) == 0);

    Dict dict;
    copy(&handle, dict.insert("a"));
    copy(&handle, dict.insert("b"));

    test_assert(count_references_to_pointer(&dict, &myobject) == 2);

    // Build a tree of depth 3, at each depth there are 4 branches where one is a
    // nested list and the other 3 are handles, except for the last depth which
    // is just 4 handles.
    List tree;
    TaggedValue* head = &tree;
    for (int depth=0; depth < 4; depth++) {
        List& list = *List::cast(head, 4);
        copy(&handle, list[0]);
        copy(&handle, list[1]);
        copy(&handle, list[2]);
        copy(&handle, list[3]);
        head = list[3];
    }

    test_equals(count_references_to_pointer(&tree, &myobject), 13);
}

void register_tests()
{
    REGISTER_TEST_CASE(gc_tests::test_visit_heap);
    REGISTER_TEST_CASE(gc_tests::test_count_references);
}

} // namesapce gc_tests
} // namespace circa
