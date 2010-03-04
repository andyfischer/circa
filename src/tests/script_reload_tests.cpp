// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace script_reload_tests {

struct ReloadHelper
{
    ReloadHelper()
    {
        branch.compile("include('file')");
    }

    void load(std::string const text)
    {
        files["file"] = text;
        files.last_modified("file")++;
        rerun();
    }

    void rerun()
    {
        Term errorListener;
        evaluate_branch(branch, &errorListener);
        if (has_runtime_error(&errorListener)) {
            std::cout << "Error in reload test: " << get_current_test_name() << std::endl;
            std::cout << get_error_message(branch) << std::endl;
            dump_branch(branch);
            declare_current_test_failed();
        }
    }

    Branch branch;
    FakeFileSystem files;
};

void test_simple()
{
    ReloadHelper helper;
    helper.load("a = 1");
    helper.load("a = 2");
}

void test_simple_with_state()
{
    ReloadHelper helper;
    helper.load("state List l; l.append(1)");
    helper.load("state List l; l.append(2)");
    test_assert(as_int(as_branch(helper.branch["l"])[0]) == 1);
    test_assert(as_int(as_branch(helper.branch["l"])[1]) == 2);
}

void test_function_change()
{
    ReloadHelper helper;
    helper.load("def f(int i); add(i,i) end; f(5)");
    helper.load("def f(number i); mult(i,i+1) end; f(5)");
}

void test_with_custom_type()
{
    ReloadHelper helper;
    helper.load("type T { int i }; state List l; l.append([1] -> T)");
    helper.load("type T { int i }; state List l; l.append([1] -> T); l.append([1] -> T)");
}

void register_tests()
{
    REGISTER_TEST_CASE(script_reload_tests::test_simple);
    REGISTER_TEST_CASE(script_reload_tests::test_simple_with_state);
    REGISTER_TEST_CASE(script_reload_tests::test_function_change);
    REGISTER_TEST_CASE(script_reload_tests::test_with_custom_type);
}

} // namespace script_reload_tests
} // namespace circa
