// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "function.h"
#include "interpreter.h"
#include "names_builtin.h"
#include "symbols.h"

namespace symbol_test {

void builtin_to_string()
{
    test_equals(builtin_symbol_to_string(s_failure), "Failure");
    test_equals(builtin_symbol_to_string(s_file), "File");
    test_equals(builtin_symbol_to_string(s_unknown), "Unknown");
}

void find_builtin_with_string()
{
    Value value;
    set_symbol_from_string(&value, "Unknown");
    test_equals(as_symbol(&value), s_unknown);

    test_equals(builtin_symbol_from_string("Unknown"), s_unknown);
    test_equals(builtin_symbol_from_string("UnknownIdentifier"), s_UnknownIdentifier);
    test_equals(builtin_symbol_from_string("Repeat"), s_repeat);
    test_equals(builtin_symbol_from_string("Success"), s_success);
    test_equals(builtin_symbol_from_string("File"), s_file);
    test_equals(builtin_symbol_from_string("FileNotFound"), s_FileNotFound);

    test_equals(builtin_symbol_from_string("Nonexistant123"), -1);
    test_equals(builtin_symbol_from_string("Fi"), -1);
    test_equals(builtin_symbol_from_string("FileN"), -1);
    test_equals(builtin_symbol_from_string("File "), -1);
}

void new_runtime_symbol()
{
    Value a, b, c;
    set_symbol_from_string(&a, "NewSymbol");
    set_symbol_from_string(&b, "NewSymbol");
    set_symbol_from_string(&c, "NewSymbol2");

    test_assert(as_symbol(&a) == as_symbol(&b));
    test_assert(as_symbol(&a) != as_symbol(&c));
    test_assert(as_symbol(&b) != as_symbol(&c));
}

void builtin_symbol_used_in_code()
{
    Block block;
    Block* f = nested_contents(block.compile("def f() -> Symbol { :Unknown }"));

    Stack stack;
    stack_init(&stack, f);
    circa_run(&stack);

    test_equals(circa_output(&stack, 0), ":Unknown");
    test_equals(as_symbol(circa_output(&stack, 0)), s_unknown);
}

void register_tests()
{
    REGISTER_TEST_CASE(symbol_test::builtin_to_string);
    REGISTER_TEST_CASE(symbol_test::find_builtin_with_string);
    REGISTER_TEST_CASE(symbol_test::new_runtime_symbol);
    REGISTER_TEST_CASE(symbol_test::builtin_symbol_used_in_code);
}

} // namespace symbol_test
