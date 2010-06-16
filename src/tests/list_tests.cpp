// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "tagged_value_vector.h"

namespace circa {
namespace list_tests {

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
        make_int(((List*) &value)->append(), 0);

        tagged_value_vector::ListData* origData = (tagged_value_vector::ListData*)
            get_pointer(&value);
        test_assert(tagged_value_vector::refcount(origData) == 1);

        // Keep a ref on origData, so that it's not deleted
        tagged_value_vector::incref(origData);
        test_assert(tagged_value_vector::refcount(origData) == 2);

        operation((List*) &value);

        // If value has the same data, then refcount should be unchanged.
        // If value has new data, then the original data should be down
        // to 1 refcount.

        tagged_value_vector::ListData* newData = (tagged_value_vector::ListData*)
            get_pointer(&value);
        if (origData == newData) {
            test_assert(tagged_value_vector::refcount(origData) == 2);
        } else {
            test_assert(tagged_value_vector::refcount(origData) == 1);
            test_assert(tagged_value_vector::refcount(newData) == 1);
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
    create_int(as_branch(a), 4);
    create_string(as_branch(a), "hello");

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

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::test_equals_branch);
    REGISTER_TEST_CASE(list_tests::test_tagged_value);
    REGISTER_TEST_CASE(list_tests::memory_management_for_each_operation::run_all);
}

}
}
