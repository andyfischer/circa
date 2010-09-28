// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace list_tests {

#if 0
namespace memory_management_for_each_operation
{
    typedef void (*ListOperation)(List* value);

    void set_index_op(List* value)
    {
        TaggedValue one;
        make_int(&one, 1);
        value->set(0, &one);
    }

    void get_index_op(List* value)
    {
        value->get(0);
    }

    void append_op(List* value)
    {
        value->append();
    }

    void run_test(ListOperation operation, const char* name)
    {
        TaggedValue value;
        make_list(&value);
        make_int(List::checkCast(&value)->append(), 0);

        tvvector::ListData* origData = (tvvector::ListData*)
            get_pointer(&value);
        test_assert(tvvector::refcount(origData) == 1);

        // Keep a ref on origData, so that it's not deleted
        tvvector::incref(origData);
        test_assert(tvvector::refcount(origData) == 2);

        operation(List::checkCast(&value));

        // If value has the same data, then refcount should be unchanged.
        // If value has new data, then the original data should be down
        // to 1 refcount.

        tvvector::ListData* newData = (tvvector::ListData*)
            get_pointer(&value);
        if (origData == newData) {
            test_assert(tvvector::refcount(origData) == 2);
        } else {
            test_assert(tvvector::refcount(origData) == 1);
            test_assert(tvvector::refcount(newData) == 1);
        }
    }

    void run_all()
    {
        run_test(set_index_op, "set_index");
        run_test(get_index_op, "get_index");
        run_test(append_op, "append");
    }
}

void test_equals_branch()
{
    Branch branch;

    Term* a = create_branch(branch).owningTerm;
    create_int(a->nestedContents, 4);
    create_string(a->nestedContents, "hello");

    Term* b = branch.eval("[4 'hello']");
    Term* c = branch.eval("[4 'bye']");
    Term* d = branch.eval("[4]");
    Term* e = branch.eval("[]");

    test_assert(equals(a,b));
    test_assert(equals(b,a));
    test_assert(!equals(a,c));
    test_assert(!equals(a,d));
    test_assert(!equals(a,e));
}

void test_tagged_value()
{
    Branch branch;

    branch.eval("type MyType { string s, int i }");
    Term* val = branch.eval("MyType()");

    test_assert(is_string(val->getIndex(0)));
    test_assert(is_int(val->getIndex(1)));

    test_assert(is_string(val->getField("s")));
    test_assert(is_int(val->getField("i")));
    test_assert(val->getField("x") == NULL);
}

#endif
void register_tests()
{
    // Unsupported:
    //REGISTER_TEST_CASE(list_tests::test_equals_branch);

    //TEST_DISABLED REGISTER_TEST_CASE(list_tests::test_tagged_value);
    //TEST_DISABLED REGISTER_TEST_CASE(list_tests::memory_management_for_each_operation::run_all);
}

}
}
