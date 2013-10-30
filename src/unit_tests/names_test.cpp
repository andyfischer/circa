// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "inspection.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "string_type.h"

namespace names_test {

void unique_ordinals()
{
    Block block;
    Term* a = block.compile("a = 1");
    Term* b = block.compile("b = 1");

    test_equals(a->uniqueOrdinal, 0);
    test_equals(b->uniqueOrdinal, 0);

    Term* a2 = block.compile("a = 3");
    test_equals(a->uniqueOrdinal, 1);
    test_equals(a2->uniqueOrdinal, 2);

    Term* a3 = block.compile("a = 3");
    test_equals(a3->uniqueOrdinal, 3);

    Term* x = block.compile("x = 4");
    test_equals(x->uniqueOrdinal, 0);
    rename(x, "a");
    test_equals(x->uniqueOrdinal, 4);
}

void global_names()
{
    Block* block = find_or_create_block(global_root_block(), "names_test");

    Term* a = block->compile("a = 1");

    circa::Value globalName;
    get_global_name(a, &globalName);
    test_equals(&globalName, "names_test:a");

    Term* a_2 = block->compile("a = 2");
    get_global_name(a, &globalName);
    test_equals(&globalName, "names_test:a#1");

    get_global_name(a_2, &globalName);
    test_equals(&globalName, "names_test:a#2");

    Block* block2 = create_block(block, "block2");
    Term* b = block2->compile("b = 3");
    Term* b_2 = block2->compile("b = 4");

    get_global_name(b, &globalName);
    test_equals(&globalName, "names_test:block2:b#1");
    get_global_name(b_2, &globalName);
    test_equals(&globalName, "names_test:block2:b#2");

    // Now try finding terms via their global name.
    test_assert(find_from_global_name(global_world(), "names_test:a") == a_2);
    test_assert(find_from_global_name(global_world(), "names_test:a#1") == a);
    test_assert(find_from_global_name(global_world(), "names_test:a#2") == a_2);
    test_assert(find_from_global_name(global_world(), "names_test:block2:b") == b_2);
    test_assert(find_from_global_name(global_world(), "names_test:block2:b#1") == b);
    test_assert(find_from_global_name(global_world(), "names_test:block2:b#2") == b_2);
}

void test_find_ordinal_suffix()
{
    int endPos;

    endPos = 1;
    test_equals(name_find_ordinal_suffix("a", &endPos), -1);
    test_equals(endPos, 1);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a#2", &endPos), 2);
    test_equals(endPos, 1);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a##", &endPos), -1);
    test_equals(endPos, 3);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a12", &endPos), -1);
    test_equals(endPos, 3);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("#12", &endPos), 12);
    test_equals(endPos, 0);

    endPos = 7;
    test_equals(name_find_ordinal_suffix("a#2:b#3", &endPos), 3);
    test_equals(endPos, 5);

    endPos = 3;
    test_equals(name_find_ordinal_suffix("a#2:b#3", &endPos), 2);
    test_equals(endPos, 1);
    
    // test using -1 as the end position
    endPos = -1;
    test_equals(name_find_ordinal_suffix("a#2", &endPos), 2);
    test_equals(endPos, 1);
}

void search_every_global_name()
{
    // This test is brave. We go through every single term in the world, find its
    // global name (if it exists), then see if we can find the original term using
    // the global name.

    circa::Value globalName;
    for (BlockIterator it(global_root_block()); it.unfinished(); it.advance()) {
        get_global_name(*it, &globalName);

        if (!is_string(&globalName))
            continue;

        Term* searchResult = find_from_global_name(global_world(), as_cstring(&globalName));

        if (searchResult != *it) {
            std::cout << "Global name search failed for term: " << global_id(*it)
                << ", with global name: " << as_cstring(&globalName) << std::endl;
            declare_current_test_failed();
        }
    }
}

void bug_with_lookup_type_and_qualified_name()
{
    // Bug repro. There was an issue where, when searching for a qualified name, we would
    // use the original lookup type on the prefix. (which is wrong).

    Block block;
    Block* module = create_block(&block, "module");
    Term* T = create_type(module, "T");

    test_assert(T == find_name(&block, "module:T", sym_LookupType));
}

void type_name_visible_from_module()
{
    test_write_fake_file("a", 1, "type A { int i }");
    load_module_file(global_world(), temp_string("a"), "a");

    test_write_fake_file("b", 1, "require a\ntest_spy(make(A))");
    Block* b = load_module_file(global_world(), temp_string("b"), "b");

    Stack stack;
    stack_init(&stack, b);
    test_spy_clear();
    run_interpreter(&stack);
    test_assert(&stack);

    test_equals(test_spy_get_results(), "[{i: 0}]");
}

void register_tests()
{
    REGISTER_TEST_CASE(names_test::global_names);
    REGISTER_TEST_CASE(names_test::unique_ordinals);
    REGISTER_TEST_CASE(names_test::test_find_ordinal_suffix);
    REGISTER_TEST_CASE(names_test::search_every_global_name);
    REGISTER_TEST_CASE(names_test::bug_with_lookup_type_and_qualified_name);
    REGISTER_TEST_CASE(names_test::type_name_visible_from_module);
}

} // namespace names
