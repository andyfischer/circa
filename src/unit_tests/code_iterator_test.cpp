// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "code_iterators.h"
#include "debug.h"
#include "inspection.h"

namespace code_iterator_test {

void block_iterator_2()
{
    Block block;
    block.compile("a = 1; b = 2; c = section { d = 3; e = 4; } f = 5");

    BlockIterator2 it(&block);
    test_assert(it.current()->name == "a"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "b"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "c"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "d"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "e"); ++it; test_assert(!it.finished());
    test_assert(it.current()->name == "f"); ++it; test_assert(it.finished());
}

void block_iterator_2_2()
{
    Block block;
    block.compile("a = section { b = section { c = section { d = 3; e = 4; }}}");

    BlockIterator2 it(&block);
    test_assert(it.current()->name == "a"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "b"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "c"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "d"); test_assert(!it.finished()); ++it;
    test_assert(it.current()->name == "e"); test_assert(!it.finished()); ++it;
    test_assert(it.finished());
}

void name_visible_iterator_1()
{
    Block block;
    block.compile("a = 1");
    Term* b = block.compile("b = 1");
    block.compile("c = 1; d = 1; e = 1");
    block.compile("b = 2; f = 1; g = 1");

    NameVisibleIterator it(b);
    test_equals(it.current()->name,"c"); test_assert(!it.finished()); ++it;
    test_equals(it.current()->name,"d"); test_assert(!it.finished()); ++it;
    test_equals(it.current()->name,"e"); test_assert(!it.finished()); ++it;
    test_assert(it.finished());
}

void upward_iterator_2()
{
    Block block;
    block.compile("a = 1; b = 1; c = 1;");
    block.compile("d = section { e = 1; f = 1; g = 1; h = section { i = 1; j = 1 } } ");
    block.compile("k = 1;");

    Value visited;

    // Start at 'a'.
    set_list(&visited);
    UpwardIterator2 it(find_term_from_path_expression(&block, "a"));
    for (; it; ++it)
        set_value(list_append(&visited), &it.current()->nameValue);
    test_equals(&visited, "['a']");

    // Start at 'f'.
    set_list(&visited);
    it = UpwardIterator2(find_term_from_path_expression(&block, "d/f"));
    for (; it; ++it)
        set_value(list_append(&visited), &it.current()->nameValue);
    test_equals(&visited, "['f', 'e', 'c', 'b', 'a']");

    // Start at 'j'.
    set_list(&visited);
    it = UpwardIterator2(find_term_from_path_expression(&block, "d/h/j"));
    for (; it; ++it)
        set_value(list_append(&visited), &it.current()->nameValue);
    test_equals(&visited, "['j', 'i', 'g', 'f', 'e', 'c', 'b', 'a']");

    // Start at 'j', stop at section 'h'.
    set_list(&visited);
    it = UpwardIterator2(find_term_from_path_expression(&block, "d/h/j"));
    it.stopAt(find_block_from_path_expression(&block, "d/h"));
    for (; it; ++it)
        set_value(list_append(&visited), &it.current()->nameValue);
    test_equals(&visited, "['j', 'i']");

    // Start at 'j', stop at section 'd'.
    set_list(&visited);
    it = UpwardIterator2(find_term_from_path_expression(&block, "d/h/j"));
    it.stopAt(find_block_from_path_expression(&block, "d"));
    for (; it; ++it)
        set_value(list_append(&visited), &it.current()->nameValue);
    test_equals(&visited, "['j', 'i', 'g', 'f', 'e']");
}

void register_tests()
{
    REGISTER_TEST_CASE(code_iterator_test::block_iterator_2);
    REGISTER_TEST_CASE(code_iterator_test::block_iterator_2_2);
    REGISTER_TEST_CASE(code_iterator_test::name_visible_iterator_1);
    REGISTER_TEST_CASE(code_iterator_test::upward_iterator_2);
}

} // namespace code_iterator_test
