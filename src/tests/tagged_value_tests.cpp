// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace tagged_value_tests {

void test_int_simple()
{
    TaggedValue v;
    set_int(&v, 4);

    test_assert(is_int(&v));
    test_assert(as_int(&v) == 4);

    Branch branch;
    Term* a = branch.eval("a = 1");
    test_assert(is_int(a));

    Term* b = branch.eval("b = a");
    test_assert(is_int(b));
}

void test_polymorphic()
{
    TaggedValue v;
    test_assert(!is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    set_int(&v, 11);
    test_assert(is_int(&v));
    test_assert(!is_float(&v));
    test_assert(!is_bool(&v));

    set_float(&v, 2.0);
    test_assert(!is_int(&v));
    test_assert(is_float(&v));
    test_assert(!is_bool(&v));

    set_bool(&v, false);
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
    assign_value(&i, &f);
    test_assert(is_float(&f));
#endif
}

void test_assign_value_to_default()
{
    Branch branch;
    Term* a = create_int(branch, 5);
    test_assert(as_int(a) == 5);
    assign_value_to_default(a);
    test_assert(as_int(a) == 0);
}

void test_constructor_syntax()
{
    Branch branch;
    Type myType;
    myType.name = "T";
    import_type(branch, &myType);
    Term* a = branch.eval("a = T()");
    test_assert(a->value_type == &myType);
    test_assert(a->value_data.ptr == NULL);
    assign_value_to_default(a);
    test_assert(a->value_type == &myType);
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

    void destroy(Type* type, TaggedValue* value)
    {
        pool_deallocate(value->value_data.asint);
    }

    void test()
    {
        initialize_pool();

        Type myType;
        myType.initialize = initialize;
        myType.destroy = destroy;

        TaggedValue value;

        test_assert(is_null(&value));
        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        change_type(&value, &myType);

        test_assert(pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        set_null(&value);

        test_assert(!pool_allocated[0]);
        test_assert(!pool_allocated[1]);

        // scope 1:
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, &myType);
            test_assert(pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);

        // scope 2
        {
            TaggedValue scoped_value;
            change_type(&scoped_value, &myType);
            test_assert(pool_allocated[0]);
            set_null(&scoped_value);
            test_assert(!pool_allocated[0]);
        }
        test_assert(!pool_allocated[0]);
    }
}

namespace refcount_test {

    // toy memory pool:
    const int pool_size = 5;
    int refcount[pool_size];

    // helper functions for pool:
    void initialize_pool()
    {
        for (int i=0; i < pool_size; i++)
            refcount[i] = 0;
    }

    int pool_allocate()
    {
        for (int i=0; i < pool_size; i++) {
            if (refcount[i] == 0) {
                refcount[i]++;
                return i;
            }
        }
        assert(false);
        return -1;
    }

    void pool_deallocate(int index)
    {
        assert(refcount[index] > 0);
        refcount[index]--;
    }

    // Type functions:
    void initialize(Type* type, TaggedValue* value)
    {
        value->value_data.asint = pool_allocate();
    }

    void destroy(Type* type, TaggedValue* value)
    {
        pool_deallocate(value->value_data.asint);
    }

    void assign(TaggedValue* source, TaggedValue* dest)
    {
        int prev = dest->value_data.asint;
        dest->value_data.asint = source->value_data.asint;
        refcount[dest->value_data.asint]++;
        refcount[prev]--;
    }

    void test()
    {
        Type myType;
        myType.initialize = initialize;
        myType.destroy = destroy;
        myType.assign = assign;

        {
            TaggedValue value;
            change_type(&value, &myType);

            test_assert(refcount[0] == 1);
        }

        test_assert(refcount[0] == 0);

        {
            TaggedValue value1, value2;
            change_type(&value1, &myType);
            change_type(&value2, &myType);

            test_assert(refcount[0] == 1);

            assign_value(&value1, &value2);

            test_assert(value1.value_data.asint == 0);
            test_assert(value2.value_data.asint == 0);

            test_assert(refcount[0] == 2);
        }

        test_assert(refcount[0] == 0);
    }
}

void register_tests()
{
    REGISTER_TEST_CASE(tagged_value_tests::test_int_simple);
    REGISTER_TEST_CASE(tagged_value_tests::test_polymorphic);
    REGISTER_TEST_CASE(tagged_value_tests::test_term_value);
    REGISTER_TEST_CASE(tagged_value_tests::subroutine_call_test);
    REGISTER_TEST_CASE(tagged_value_tests::test_float);
    REGISTER_TEST_CASE(tagged_value_tests::test_assign_value_to_default);
    REGISTER_TEST_CASE(tagged_value_tests::test_constructor_syntax);
    REGISTER_TEST_CASE(tagged_value_tests::manual_memory_management_test::test);
    REGISTER_TEST_CASE(tagged_value_tests::refcount_test::test);
}

}
}
