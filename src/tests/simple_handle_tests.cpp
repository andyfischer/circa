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

    TaggedValue handle;

    assign(&handle, 0);
    test_equals(&slots, "[true, false, false]");

    set_null(&handle);

    test_equals(&slots, "[false, false, false]");

    assign(&handle, 0);
    test_equals(&slots, "[true, false, false]");
    assign(&handle, 1);
    test_equals(&slots, "[false, true, false]");
    assign(&handle, 2);
    test_equals(&slots, "[false, false, true]");

    TaggedValue handle2;
    TaggedValue handle3;
    assign(&handle2, 0);
    test_equals(&slots, "[true, false, true]");
    assign(&handle3, 1);
    test_equals(&slots, "[true, true, true]");
    set_null(&handle2);
    test_equals(&slots, "[false, true, true]");
    set_null(&handle);
    test_equals(&slots, "[false, true, false]");
}

CA_FUNCTION(alloc_handle)
{
    if (INPUT(0)->value_type != &handle_type)
        assign(OUTPUT, 0);
    else
        copy(INPUT(0), OUTPUT);
}

void test_with_state()
{
    setup();
    test_equals(&slots, "[false, false, false]");

    Branch branch;
    branch.compile("def alloc_handle(any s) -> any;");
    install_function(branch["alloc_handle"], alloc_handle);

    branch.compile("state s");
    branch.compile("alloc_handle(@s)");

    EvalContext context;
    evaluate_branch_no_preserve_locals(&context, branch);

    test_equals(&slots, "[true, false, false]");

    evaluate_branch_no_preserve_locals(&context, branch);
    test_equals(&slots, "[true, false, false]");

    reset(&context.state);
    reset_locals(branch);
    test_equals(&slots, "[false, false, false]");
}

void register_tests()
{
    REGISTER_TEST_CASE(simple_handle_tests::test_simple);
    REGISTER_TEST_CASE(simple_handle_tests::test_with_state);
}

}
}
