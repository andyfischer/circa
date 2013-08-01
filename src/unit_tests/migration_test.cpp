// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "building.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "migration.h"
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

    test_assert(b2 == migrate_term_pointer(b1, &block1, &block2));
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

    migrate_block(&target, &version1, &version2);

    test_assert(call->function != version1.get("f"));
    test_assert(call->function == version2.get("f"));
}

void term_ref_values()
{
#if 0
    Block version1;
    Term* f1 = version1.compile("def f()");

    Block version2;
    Term* f2 = version2.compile("def f()");

    version1.compile("state br = f.block");
    version1.compile("state tr = br.owner");

    Stack* stack = create_stack(global_world());

    stack_init(stack, &version1);
    run_interpreter(stack);
    stack_restart(stack);

    caValue* state = stack_find_state_input_register(stack);
    test_assert(as_term_ref(get_field(state, "tr")) == f1);
    test_assert(as_block(get_field(state, "br")) == f1->nestedContents);

    migrate_value(state, &version1, &version2);

    test_assert(as_term_ref(get_field(state, "tr")) == f2);
    test_assert(as_block(get_field(state, "br")) == f2->nestedContents);

    free_stack(stack);
#endif
}

void stack_value()
{
#if 0
    Block version1;
    Term* f1 = version1.compile("def f()");
    version1.compile("state int = make_stack()");
    version1.compile("int.stack_push(f.block, [])");

    Block version2;
    Term* f2 = version2.compile("def f()");

    Stack* stack = create_stack(global_world());
    stack_init(stack, &version1);
    run_interpreter(stack);
    stack_restart(stack);

    caValue* state = stack_find_state_input_register(stack);

    Stack* runtimeStack = as_stack(get_field(state, "int"));
    test_assert(frame_block(stack_top(runtimeStack)) == f1->nestedContents);

    migrate_value(state, &version1, &version2);

    runtimeStack = as_stack(get_field(state, "int"));
    test_assert(frame_block(stack_top(runtimeStack)) == f2->nestedContents);

    free_stack(stack);
    #endif
}

void run_and_migrate_and_run(const char* script1, const char* script2)
{
    Block block1;
    block1.compile(script1);

    Stack stack;
    stack_init(&stack, &block1);
    run_interpreter(&stack);

    Block block2;
    block2.compile(script2);

    migrate_stack(&stack, &block1, &block2);

    stack_restart(&stack);
    run_interpreter(&stack);
}

void mutable_value()
{
    test_spy_clear();

    run_and_migrate_and_run(
        "def f() -> int { 5 }\n"
        "state Mutable val = make(Mutable)\n"
        "val.set(f)\n",

        "def f() -> int { 7 }\n"
        "state Mutable val\n"
        "cl = Func(val.get)\n"
        "test_spy(cl.call())\n"
        );

    test_equals(test_spy_get_results(), "[7]");
}

void translate_terms_type()
{
    // Setup
    test_write_fake_file("lib.ca", 1, "type T { int a }");
    Block* lib = load_module_file(global_world(), "translate_terms_type_lib", "lib.ca");
    Type* T = find_type_local(lib, "T");

    Block block;
    block.compile("require translate_terms_type_lib\nt = make(T)");
    
    test_assert(block["t"]->type == T);

    test_write_fake_file("lib.ca", 2, "type T { float a }");
    Block* newLib = load_module_file(global_world(), "translate_terms_type_lib", "lib.ca");
    Type* newT = find_type_local(newLib, "T");
    test_assert(T != newT);

    migrate_block(&block, lib, newLib);
    test_assert(block["t"]->type == newT);
}

void bug_with_migration_and_stale_pointer()
{
    test_spy_clear();

    run_and_migrate_and_run(
        "type T { int a }\n"
        "def make_t() -> T { make(T) }\n"
        "def inc_t(T t) -> T { t.a += 1 }\n"
        "test_spy(inc_t(make_t()))",

        "type T { int a }\n"
        "def make_t() -> T { make(T) }\n"
        "def inc_t(T t) -> T { t.a += 2}"
        "test_spy(inc_t(make_t()))"
    );

    test_equals(test_spy_get_results(), "[{a: 1}, {a: 2}]");
}

void stack_migration_deletes_block()
{
    Block block;
    block.compile("a = 1 + 2");

    Stack stack;
    stack_init(&stack, &block);

    // This once crashed:
    migrate_stack(&stack, &block, NULL);
}

void register_tests()
{
#if 0 // Old migration DISABLED
    REGISTER_TEST_CASE(migration_test::translate_terms);
    REGISTER_TEST_CASE(migration_test::update_references);
    REGISTER_TEST_CASE(migration_test::term_ref_values);
    REGISTER_TEST_CASE(migration_test::stack_value);
    REGISTER_TEST_CASE(migration_test::mutable_value);
    REGISTER_TEST_CASE(migration_test::translate_terms_type);
    REGISTER_TEST_CASE(migration_test::bug_with_migration_and_stale_pointer);
    REGISTER_TEST_CASE(migration_test::stack_migration_deletes_block);
#endif
}

}
