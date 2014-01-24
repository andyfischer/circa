// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "kernel.h"
#include "stack.h"
#include "tagged_value.h"
#include "world.h"

namespace stack_test {

void resize_frame_test()
{
    Stack* stack = create_stack(global_world());

    stack_push_blank_frame(stack, 5);

    test_assert(frame_register_count(top_frame(stack)) == 5);
    for (int i=0; i < 5; i++)
        set_int(frame_register(top_frame(stack), i), i);

    stack_resize_frame(stack, top_frame(stack), 7);

    for (int i=0; i < 5; i++)
        test_equals(frame_register(top_frame(stack), i), i);

    test_equals(frame_register(top_frame(stack), 5), "null");
    test_equals(frame_register(top_frame(stack), 6), "null");

    stack_resize_frame(stack, top_frame(stack), 2);

    for (int i=0; i < 2; i++)
        test_equals(frame_register(top_frame(stack), i), i);
}

void resize_frame_test2()
{
    Stack* stack = create_stack(global_world());

    stack_push_blank_frame(stack, 5);
    stack_push_blank_frame(stack, 10);

    Frame* frame1 = first_frame(stack);
    Frame* frame2 = top_frame(stack);

    for (int i=0; i < 5; i++)
        set_int(frame_register(frame1, i), i);
    for (int i=0; i < 10; i++)
        set_int(frame_register(frame2, i), i + 100);

    frame1 = stack_resize_frame(stack, frame1, 20);

    test_assert(first_frame(stack) == frame1);
    test_assert(prev_frame(top_frame(stack)) == frame1);
    test_assert(next_frame(frame1) == top_frame(stack));
    test_assert(next_frame(top_frame(stack)) == NULL);

    for (int i=0; i < 5; i++)
        test_equals(frame_register(frame1, i), i);
    for (int i=5; i < 15; i++)
        test_equals(frame_register(frame1, i), "null");
    for (int i=0; i < 10; i++)
        test_equals(frame_register(top_frame(stack), i), i + 100);
}

void register_tests()
{
    REGISTER_TEST_CASE(stack_test::resize_frame_test);
    REGISTER_TEST_CASE(stack_test::resize_frame_test2);
}

} // namespace stack_test
