// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace tagged_value_tests {

namespace toy_refcounted_pool {
    const int pool_size = 5;
    int refcount[pool_size];

    // helper functions for pool:
    void initialize_pool()
    {
        for (int i=0; i < pool_size; i++)
            refcount[i] = 0;
    }

    bool nothing_allocated()
    {
        for (int i=0; i < pool_size; i++)
            if (refcount[i] != 0)
                return false;
        return true;
    }

    void initialize(Type*, TaggedValue* value)
    {
        for (int i=0; i < pool_size; i++) {
            if (refcount[i] == 0) {
                refcount[i]++;
                value->value_data.asint = i;
                return;
            }
        }
        assert(false);
    }

    void release(TaggedValue* value)
    {
        int index = value->value_data.asint;
        assert(refcount[index] > 0);
        refcount[index]--;
    }

    void copy(TaggedValue* source, TaggedValue* dest)
    {
        int prev = dest->value_data.asint;
        dest->value_data.asint = source->value_data.asint;
        refcount[dest->value_data.asint]++;
        refcount[prev]--;
    }
    void setup_type(Type* type)
    {
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
    }
}


void test_int_simple()
{
    TaggedValue v;
    make_int(&v, 4);

    test_assert(is_int(&v));
    test_assert(as_int(&v) == 4);

    Branch branch;
    Term* a = branch.eval("a = 1");
    test_assert(is_int(a));

    Term* b = branch.eval("b = a");
    test_assert(is_int(b));
}

void test_int_and_float_casting()
{
    TaggedValue i;
    make_int(&i, 4);
    TaggedValue f;
    make_float(&f, 0.0);

    cast(f.value_type, &i, &f);
    test_assert(as_float(&f) == 4.0);
}

void test_polymorphic()
{
    TaggedValue v;
    test_assert(!is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    make_int(&v, 11);
    test_assert(is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    make_float(&v, 2.0);
    test_assert(!is_int(&v));
    test_assert(is_float(&v));
    test_assert(!is_bool(&v));

    make_bool(&v, false);
    test_assert(!is_int(&v));
    test_assert(!is_float(&v));
    test_assert(is_bool(&v));
}

void test_term_value()
{
    Branch branch;
    Term* i = create_int(branch, 5);
    test_assert(is_int(i));
    test_assert(is_int(i));

    Term* a = branch.eval("a = [1 2 3]");
    test_assert(is_int(a->asBranch()[0]));

    Term* b = branch.eval("b = a");
    test_assert(is_int(b->asBranch()[0]));
    
    Term* c = create_value(branch, INT_TYPE);
    test_assert(is_int(c));

    Branch list;
    resize_list(list, 4, INT_TYPE);
    test_assert(is_int(list[0]));
}

namespace subroutine_call_test_helper {
    void assert_ints(EvalContext*, Term* term)
    {
        test_assert(is_int(term->input(0)));
        test_assert(is_int(term));
    }
}

void subroutine_call_test()
{
    Branch branch;
    import_function(branch, subroutine_call_test_helper::assert_ints, "f(int) -> int");
    branch.eval("f(1)");
}

void test_float()
{
#if 0
    TaggedValue f = tag_float(5);
    TaggedValue i = tag_int(5);
    test_assert(is_int(&i));
    test_assert(is_float(&f));
    copy(&i, &f);
    test_assert(is_float(&f));
#endif
}

void test_reset()
{
    Branch branch;
    Term* a = create_int(branch, 5);
    test_assert(as_int(a) == 5);
    reset(a);
    test_assert(as_int(a) == 0);
}

void test_constructor_syntax()
{
    Branch branch;
    TypeRef myType = Type::create();
    myType->name = "T";
    import_type(branch, myType);
    Term* a = branch.eval("a = T()");
    test_assert(a->value_type == myType);
    test_assert(a->value_data.ptr == NULL);
    reset(a);
    test_assert(a->value_type == myType);
    test_assert(a->value_data.ptr == NULL);
}

namespace manual_memory_management_test {
    
    // toy memory pool:
    const int pool_size = 5;
    bool pool_allocated[pool_size];

    // helper functions for pool:
    void initialize_pool()
    {
        for (int i=0; i < pool_size; i++)
            pool_allocated[i] = false;
    }

    int pool_allocate()
    {
        for (int i=0; i < pool_size; i++) {
            if (!pool_allocated[i]) {
                pool_allocated[i] = true;
                return i;
            }
        }
        assert(false);
        return -1;
    }

    void pool_deallocate(int index)
    {
        assert(pool_allocated[index]);
        pool_allocated[index] = false;
    }

    // Type functions:
    void initialize(Type* type, TaggedValue* value)
    {
        value->value_data.asint = pool_allocate();
    }

    void release(TaggedValue* value)
    {
        pool_deallocate(value->value_data.asint);
    }

    void test()
    {
        initialize_pool();

        TypeRef myType = Type::create();
        myType->initialize = initialize;
        myType->release = release;

        TaggedValue value;

        test_assert(is_null(&value));
        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        change_type(&value, myType);

        test_assert(pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        set_null(&value);

        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        // scope 1:
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, myType);
            test_assert(pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);

        // scope 2
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, myType);
            test_assert(pool_allocated[0]);
            set_null(&scoped_value);
            test_assert(!pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);
    }
}

void refcount_test()
{
    TypeRef t = Type::create();
    toy_refcounted_pool::setup_type(t);
    toy_refcounted_pool::initialize_pool();

    {
        TaggedValue value(t);

        test_assert(toy_refcounted_pool::refcount[0] == 1);
    }

    test_assert(toy_refcounted_pool::nothing_allocated());

    {
        TaggedValue value1(t), value2(t);

        test_assert(toy_refcounted_pool::refcount[0] == 1);

        copy(&value1, &value2);

        test_assert(value1.value_data.asint == 0);
        test_assert(value2.value_data.asint == 0);

        test_assert(toy_refcounted_pool::refcount[0] == 2);
    }

    test_assert(toy_refcounted_pool::nothing_allocated());
}

void list_memory_management()
{
    List list;
    TypeRef t = Type::create();
    toy_refcounted_pool::setup_type(t);
    toy_refcounted_pool::initialize_pool();

    TaggedValue v(t);

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    copy(&v, list.append());

    test_assert(toy_refcounted_pool::refcount[0] == 2);

    list.clear();

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    swap(&v, list.append());

    test_assert(toy_refcounted_pool::refcount[0] == 1);

    list.clear();

    test_assert(toy_refcounted_pool::refcount[0] == 0);

    change_type(list.append(), t);
    change_type(list.append(), t);
    change_type(list.append(), t);

    test_assert(toy_refcounted_pool::refcount[0] == 1);
    test_assert(toy_refcounted_pool::refcount[1] == 1);
    test_assert(toy_refcounted_pool::refcount[2] == 1);

    make_null(list[1]);

    test_assert(toy_refcounted_pool::refcount[0] == 1);
    test_assert(toy_refcounted_pool::refcount[1] == 0);
    test_assert(toy_refcounted_pool::refcount[2] == 1);

    copy(list[2], list[0]);

    test_assert(toy_refcounted_pool::refcount[0] == 0);
    test_assert(toy_refcounted_pool::refcount[1] == 0);
    test_assert(toy_refcounted_pool::refcount[2] == 2);

    list.clear();

    test_assert(toy_refcounted_pool::nothing_allocated());
}

void register_tests()
{
    REGISTER_TEST_CASE(tagged_value_tests::test_int_simple);
    REGISTER_TEST_CASE(tagged_value_tests::test_int_and_float_casting);
    REGISTER_TEST_CASE(tagged_value_tests::test_polymorphic);
    REGISTER_TEST_CASE(tagged_value_tests::test_term_value);
    REGISTER_TEST_CASE(tagged_value_tests::subroutine_call_test);
    REGISTER_TEST_CASE(tagged_value_tests::test_float);
    REGISTER_TEST_CASE(tagged_value_tests::test_reset);
    REGISTER_TEST_CASE(tagged_value_tests::test_constructor_syntax);
    REGISTER_TEST_CASE(tagged_value_tests::manual_memory_management_test::test);
    REGISTER_TEST_CASE(tagged_value_tests::refcount_test);
    REGISTER_TEST_CASE(tagged_value_tests::list_memory_management);
}

}
} // namespace circa
