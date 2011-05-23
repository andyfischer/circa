// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"

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

void register_tests()
{
    REGISTER_TEST_CASE(gc_tests::test_visit_heap);
}

} // namesapce gc_tests
} // namespace circa
