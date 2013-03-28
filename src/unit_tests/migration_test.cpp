// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "actors.h"
#include "block.h"
#include "building.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "modules.h"
#include "reflection.h"
#include "world.h"

namespace migration_test {

void translate_terms()
{
    Block block1;
    Block block2;

    block1.compile("a = 1");
    Term* b1 = block1.compile("b = 2");
    block1.compile("c = 3");

    block2.compile("a = 1");
    Term* b2 = block2.compile("b = 2");
    block2.compile("c = 3");

    test_assert(b2 == translate_term_across_blocks(b1, &block1, &block2));
}

void update_references()
{
    Block version1;
    version1.compile("def f()");
    Block version2;
    version2.compile("def f()");

    Block target;
    Term* call = target.compile("f()");
    change_function(call, version1.get("f"));
    test_assert(call->function == version1.get("f"));

    update_all_code_references(&target, &version1, &version2);

    test_assert(call->function != version1.get("f"));
    test_assert(call->function == version2.get("f"));
}

void term_ref_values()
{
    Block version1;
    Term* f1 = version1.compile("def f()");

    Block version2;
    Term* f2 = version2.compile("def f()");

    version1.compile("state tr = term_ref(f)");
    version1.compile("state br = tr.contents");

    Stack* stack = create_stack(global_world());

    push_frame(stack, &version1);
    run_interpreter(stack);
    stack_restart(stack);

    caValue* state = stack_find_state_input_register(stack);
    test_assert(as_term_ref(get_field(state, "tr")) == f1);
    test_assert(as_block(get_field(state, "br")) == f1->nestedContents);

    update_all_code_references_in_value(state, &version1, &version2);

    test_assert(as_term_ref(get_field(state, "tr")) == f2);
    test_assert(as_block(get_field(state, "br")) == f2->nestedContents);

    free_stack(stack);
}

void stack_value()
{
    Block version1;
    Term* f1 = version1.compile("def f()");
    version1.compile("state int = make_interpreter()");
    version1.compile("int.push_frame(f.block, [])");

    Block version2;
    Term* f2 = version2.compile("def f()");

    Stack* stack = create_stack(global_world());
    push_frame(stack, &version1);
    run_interpreter(stack);
    stack_restart(stack);

    caValue* state = stack_find_state_input_register(stack);

    Stack* runtimeStack = as_stack(get_field(state, "int"));
    test_assert(frame_block(top_frame(runtimeStack)) == f1->nestedContents);

    update_all_code_references_in_value(state, &version1, &version2);

    runtimeStack = as_stack(get_field(state, "int"));
    test_assert(frame_block(top_frame(runtimeStack)) == f2->nestedContents);

    free_stack(stack);
}

void actor_value()
{
    Block version1;
    Term* f1 = version1.compile("def f(int i)");
    version1.compile("state actor = make_actor(f.block)");

    Block version2;
    Term* f2 = version2.compile("def f(int i)");

    Stack* stack = create_stack(global_world());
    push_frame(stack, &version1);
    run_interpreter(stack);
    stack_restart(stack);

    caValue* state = stack_find_state_input_register(stack);

    Actor* actor = as_actor(get_field(state, "actor"));
    test_assert(actor_block(actor) == f1->nestedContents);

    update_world_after_module_reload(global_world(), &version1, &version2);

    actor = as_actor(get_field(state, "actor"));
    test_assert(actor_block(actor) == f2->nestedContents);

    free_stack(stack);
}

void register_tests()
{
    REGISTER_TEST_CASE(migration_test::translate_terms);
    REGISTER_TEST_CASE(migration_test::update_references);
    REGISTER_TEST_CASE(migration_test::term_ref_values);
    REGISTER_TEST_CASE(migration_test::stack_value);
}

}
