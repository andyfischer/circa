// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "actors.h"
#include "kernel.h"

namespace actor_test {

void simple_manual()
{
    ActorSpace* space = create_actor_space(global_world());
    Actor* actor = create_actor(space);

    set_string(actor_post(actor), "hello");

    actors_run_iteration(space);

    test_assert(actor_has_incoming(actor));

    Value message;
    test_assert(actor_consume_incoming(actor, &message));
    test_equals(&message, "hello");
    set_null(&message);

    // Test actor_consume_incoming returns false when no message.
    test_assert(!actor_consume_incoming(actor, &message));

    // Try the same, sending to an address instead of a Actor*.
    caValue* address = actor_address(actor);

    set_string(actor_post(space, address), "hello");

    actors_run_iteration(space);

    test_assert(actor_has_incoming(actor));

    test_assert(actor_consume_incoming(actor, &message));
    test_equals(&message, "hello");

    free_actor_space(space);
}

void simple_script_handler()
{
    Block block;
    block.compile("msg = input(); test_spy(concat('Received: ' msg));");

    ActorSpace* space = create_actor_space(global_world());
    Actor* actor = create_actor(space);
    actor_set_block(actor, &block);

    set_int(actor_post(actor), 1);

    test_spy_clear();
    actors_run_iteration(space);

    test_equals(test_spy_get_results(), "['Received: 1']");

    set_int(actor_post(actor), 4);
    set_int(actor_post(actor), 5);
    set_int(actor_post(actor), 6);

    test_spy_clear();
    actors_run_iteration(space);

    test_equals(test_spy_get_results(), "['Received: 4', 'Received: 5', 'Received: 6']");

    free_actor_space(space);
}

void register_tests()
{
    REGISTER_TEST_CASE(actor_test::simple_manual);
    REGISTER_TEST_CASE(actor_test::simple_script_handler);
}

} // namespace actor_test
