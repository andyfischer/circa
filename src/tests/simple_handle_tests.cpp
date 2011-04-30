// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

#include "types/simple_handle.h"

namespace circa {
namespace simple_handle_tests {

const int numSlots = 3;
List slots;
Type handle_type;

void on_release_func(int handle)
{
    test_assert(as_bool(slots[handle]));
    set_bool(slots[handle], false);
}

void setup()
{
    slots.resize(numSlots);
    for (int i=0; i < numSlots; i++)
        set_bool(slots[i], false);

    simple_handle_t::setup_type(&handle_type);
    set_opaque_pointer(&handle_type.parameter, (void*) on_release_func);
}

void assign(TaggedValue* value, int handle)
{
    test_assert(!as_bool(slots[handle]));
    set_bool(slots[handle], true);
    simple_handle_t::set(&handle_type, value, handle);
}

void test_simple()
{
    setup();
    test_equals(&slots, "[false, false, false]");

    TaggedValue handle0;

    assign(&handle0, 0);
    test_equals(&slots, "[true, false, false]");

    set_null(&handle0);

    test_equals(&slots, "[false, false, false]");
}

void register_tests()
{
    REGISTER_TEST_CASE(simple_handle_tests::test_simple);
}

}
}
