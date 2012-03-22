// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circ_internal.h>

#include "filesystem.h"
#include "filesystem_dummy.h"
#include "tagged_value.h"
#include "type.h"
#include "types/handle.h"

namespace circa {
namespace handle_tests {

const int numSlots = 3;
List g_slots;
Type handle_type;

int get_free_slot()
{
    for (int i=0; i < numSlots; i++)
        if (!as_bool(g_slots[i]))
            return i;
    test_assert(false);
    return -1;
}

void on_release_func(caValue* data)
{
    int slot = as_int(data);
    test_assert(as_bool(g_slots[slot]));
    set_bool(g_slots[slot], false);
}

void assign(caValue* value, int handle)
{
    test_assert(!as_bool(g_slots[handle]));
    set_bool(g_slots[handle], true);
    caValue userdata;
    set_int(&userdata, handle);
    handle_t::set(value, &handle_type, &userdata);
}

CA_FUNCTION(alloc_handle)
{
    if (INPUT(0)->value_type != &handle_type)
        assign(OUTPUT, get_free_slot());
    else
        copy(INPUT(0), OUTPUT);
}

void setup(Branch& branch)
{
    g_slots.resize(numSlots);
    for (int i=0; i < numSlots; i++)
        set_bool(g_slots[i], false);

    handle_t::setup_type(&handle_type);
    set_opaque_pointer(&handle_type.parameter, (void*) on_release_func);

    branch.compile("def alloc_handle(any s) -> any;");
    install_function(branch["alloc_handle"], alloc_handle);
}


void test_simple()
{
    Branch branch;
    setup(branch);

    test_equals(&g_slots, "[false, false, false]");

    caValue handle;

    assign(&handle, 0);
    test_equals(&g_slots, "[true, false, false]");

    set_null(&handle);

    test_equals(&g_slots, "[false, false, false]");

    assign(&handle, 0);
    test_equals(&g_slots, "[true, false, false]");
    assign(&handle, 1);
    test_equals(&g_slots, "[false, true, false]");
    assign(&handle, 2);
    test_equals(&g_slots, "[false, false, true]");

    caValue handle2;
    caValue handle3;
    assign(&handle2, 0);
    test_equals(&g_slots, "[true, false, true]");
    assign(&handle3, 1);
    test_equals(&g_slots, "[true, true, true]");
    set_null(&handle2);
    test_equals(&g_slots, "[false, true, true]");
    set_null(&handle);
    test_equals(&g_slots, "[false, true, false]");
}


void test_with_state()
{
    Branch branch;
    setup(branch);

    test_equals(&g_slots, "[false, false, false]");

    branch.compile("state s");
    branch.compile("alloc_handle(@s)");
    set_branch_in_progress(&branch, false);

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&g_slots, "[true, false, false]");

    evaluate_branch(&context, &branch);
    test_equals(&g_slots, "[true, false, false]");

    reset(&context.state);
    test_equals(&g_slots, "[false, false, false]");
}

void test_deleted_state()
{
    Branch branch;
    setup(branch);

    branch.compile("state s");
    branch.compile("alloc_handle(@s)");

    branch.compile("state t");
    branch.compile("alloc_handle(@t)");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&g_slots, "[true, true, false]");

    clear_branch(&branch);
    branch.compile("state t");
    strip_orphaned_state(&branch, &context.state);
    
    test_equals(&g_slots, "[false, true, false]");
}

void test_in_subroutine_state()
{
    Branch branch;
    setup(branch);

    branch.compile("def hi(any input) { state s = input }");
    branch.compile("state s");
    branch.compile("alloc_handle(@s)");
    branch.compile("hi(s)");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&g_slots, "[true, false, false]");

    set_null(&context.state);

    test_equals(&g_slots, "[false, false, false]");
}

void test_state_inside_if_block()
{
    Branch branch;
    setup(branch);

    branch.compile("state s = null");
    branch.compile("if is_null(s) { s = alloc_handle(s) }");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&g_slots, "[true, false, false]");
    clear_branch(&branch);
    strip_orphaned_state(&branch, &context.state);

    test_equals(&g_slots, "[false, false, false]");
}

void test_that_stripping_state_is_recursive()
{
    Branch branch;
    setup(branch);

    branch.compile("if true { state a = 1; state s; s = alloc_handle(s) }");

    EvalContext context;
    evaluate_branch(&context, &branch);
    test_equals(&g_slots, "[true, false, false]");

    clear_branch(&branch);
    branch.compile("if true { state a = 1 }");
    strip_orphaned_state(&branch, &context.state);

    test_equals(&g_slots, "[false, false, false]");
}

void test_included_file_changed()
{
    Branch branch;
    setup(branch);

    FakeFileSystem files;
    files["f"] = "state s; alloc_handle(@s)";

    branch.compile("include('f')");

    EvalContext context;
    evaluate_branch(&context, &branch);

    test_equals(&g_slots, "[true, false, false]");

    files.set("f", "");
    evaluate_branch(&context, &branch);

    test_assert(&branch);

    test_equals(&g_slots, "[false, false, false]");
}

int MyType_allocated = 0;

struct MyType
{
    MyType() {
        MyType_allocated++;
    }
    ~MyType() {
        test_assert(MyType_allocated > 0);
        MyType_allocated--;
    }
};

void test_user_defined_type()
{
    Type* type = create_type();
    handle_t::setup_type<MyType>(type);

    // Create a value and then free it.
    test_assert(MyType_allocated == 0);

    caValue value;
    handle_t::set(&value, type, new MyType());
    test_assert(MyType_allocated == 1);

    set_null(&value);
    test_assert(MyType_allocated == 0);

    // Overwrite a value with a new value
    handle_t::set(&value, type, new MyType());
    test_assert(MyType_allocated == 1);
    handle_t::set(&value, type, new MyType());
    test_assert(MyType_allocated == 1);
    handle_t::set(&value, type, new MyType());
    test_assert(MyType_allocated == 1);
    set_null(&value);
    test_assert(MyType_allocated == 0);
};

void register_tests()
{
    REGISTER_TEST_CASE(handle_tests::test_simple);
    REGISTER_TEST_CASE(handle_tests::test_with_state);
    REGISTER_TEST_CASE(handle_tests::test_deleted_state);
    //TEST_DISABLED REGISTER_TEST_CASE(handle_tests::test_in_subroutine_state);
    //TEST_DISABLED REGISTER_TEST_CASE(handle_tests::test_state_inside_if_block);
    //TEST_DISABLED REGISTER_TEST_CASE(handle_tests::test_that_stripping_state_is_recursive);
    //TEST_DISABLED REGISTER_TEST_CASE(handle_tests::test_included_file_changed);
    REGISTER_TEST_CASE(handle_tests::test_user_defined_type);
}

}
}
