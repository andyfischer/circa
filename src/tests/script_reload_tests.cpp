// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "filesystem_dummy.h"
#include "importing_macros.h"

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
        evaluate_branch(&context, branch);
        if (context.errorOccurred) {
            std::cout << "Error in reload test: " << get_current_test_name() << std::endl;
            print_runtime_error_formatted(context, std::cout);
            dump(branch);
            declare_current_test_failed();
        }
    }

    Branch branch;
    FakeFileSystem files;
    EvalContext context;
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
    test_equals(helper.context.state.toString(), "{_include: {l: [1]}}");
    helper.load("state List l; l.append(2)");
    test_equals(helper.context.state.toString(), "{_include: {l: [1, 2]}}");
}

void test_function_change()
{
    ReloadHelper helper;
    helper.load("def f(int i) { add(i,i) } f(5)");
    helper.load("def f(number i) { mult(i,i+1) } f(5)");
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
