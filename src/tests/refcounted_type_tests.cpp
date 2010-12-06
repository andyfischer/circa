// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "refcounted_type_wrapper.h"
#include "cpp_type_wrapper.h"

namespace circa {
namespace refcounted_type_tests {

struct MyStruct
{
    int a;
    int b;
    int _refCount;

    MyStruct() : a(0), b(0) {}

    std::string toString()
    {
        std::stringstream strm;
        strm << "[" << a << ", " << b << "]";
        return strm.str();
    }
};

void test_intrusive_refcount()
{
    Type* type = Type::create();
    intrusive_refcounted::setup_type<MyStruct>(type);
    type->toString = cpp_type_wrapper::toString<MyStruct>;

    TaggedValue v;
    change_type(&v, type);

    test_equals(to_string(&v), "[0, 0]");

    MyStruct* v_value = cpp_type_wrapper::get<MyStruct>(&v);
    v_value->a = 1;
    v_value->b = 2;
    test_equals(to_string(&v), "[1, 2]");

    test_assert(v_value->_refCount == 1);

    TaggedValue v2;
    copy(&v, &v2);

    test_equals(to_string(&v2), "[1, 2]");
    test_assert(v_value->_refCount == 2);

    reset(&v2);
    test_assert(v_value->_refCount == 1);
}

namespace fake_memory {
    const int slot_count = 5;
    bool used[slot_count];

    int next_free() {
        for (int i=0; i < slot_count; i++)
            if (!used[i])
                return i;

        ca_assert(false);
        return 0;
    }

    struct Object {
        int address;
        int _refCount;

        Object()
        {
            address = next_free();
            used[address] = true;
        }
        ~Object()
        {
            used[address] = false;
        }
    };
}

void intrusive_refcount_tracked_alloc()
{
    Type *type = Type::create();
    intrusive_refcounted::setup_type<fake_memory::Object>(type);

    // Step 1: Allocate and deallocate
    test_assert(fake_memory::used[0] == false);

    TaggedValue value;
    change_type(&value, type);

    test_assert(fake_memory::used[0] == true);

    set_null(&value);

    test_assert(fake_memory::used[0] == false);

    // Step 2: Make a copy
    change_type(&value, type);

    test_assert(fake_memory::used[0] == true);
    test_assert(fake_memory::used[1] == false);

    TaggedValue value2;
    copy(&value, &value2);

    test_assert(fake_memory::used[0] == true);
    test_assert(fake_memory::used[1] == false);

    set_null(&value);

    test_assert(fake_memory::used[0] == true);
    test_assert(fake_memory::used[1] == false);
    
    set_null(&value2);

    test_assert(fake_memory::used[0] == false);
    test_assert(fake_memory::used[1] == false);
}

void register_tests()
{
    REGISTER_TEST_CASE(refcounted_type_tests::test_intrusive_refcount);
    REGISTER_TEST_CASE(refcounted_type_tests::intrusive_refcount_tracked_alloc);
}

} // refcounted_type_tests
} // namespace circa
