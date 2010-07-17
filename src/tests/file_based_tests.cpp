// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "fake_filesystem.h"

using namespace circa::storage;

namespace circa {
namespace file_based_tests {

void test_the_test()
{
    // Make sure that the file IO redirection thingy is working.
    FakeFileSystem files;

    files["a.txt"] = "hello";
    test_assert(read_text_file_as_str("a.txt") == "hello");

    write_text_file("a.txt", "goodbye");
    test_assert(read_text_file_as_str("a.txt") == "goodbye");
    test_assert(files["a.txt"] == "goodbye");
}

void test_include_function()
{
    FakeFileSystem files;

    files["file.ca"] = "a = 1";

    // Basic test, load our fake file.
    Branch branch;
    Term* incl = branch.compile("include('file.ca')");

    test_assert(is_branch(incl));
    Branch& included = as_branch(incl);

    // Make sure that the included file is loaded, even though the term wasn't evaluated.
    test_assert(included.length() > 0);

    test_assert(included["a"]->asInt() == 1);
    test_equals(included["#attr:source-file"]->asString(), "file.ca");

    // Next, modify the file and reload.
    files["file.ca"] = "b = 2";
    files.last_modified("file.ca")++;

    evaluate_term(incl);

    test_assert(!included.contains("a"));
    test_assert(included["b"]->asInt() == 2);

    // Modify the file but don't modify the last_modified time, make sure that
    // it doesn't reload this time.
    files["file.ca"] = "c = 3";
    evaluate_term(incl);
    test_assert(!included.contains("a"));
    test_assert(!included.contains("c"));
    test_assert(included["b"]->asInt() == 2);
}

void test_include_static_error_after_reload()
{
    FakeFileSystem files;
    Branch branch;

    files["file.ca"] = "add(1,1)";
    branch.compile("include('file.ca')");

    EvalContext result = evaluate_branch(branch);

    test_assert(!result.errorOccurred);

    files["file.ca"] = "add(what what)";
    files.last_modified("file.ca")++;

    result = evaluate_branch(branch);
    test_assert(result.errorOccurred);
}

void test_file_changed()
{
    Branch branch;
    FakeFileSystem files;
    files["x"] = "1";
    files["y"] = "2";

    Term* filename = branch.eval("filename = 'x'");
    Term* changed = branch.eval("file_changed(filename)");

    // First time through should always return true
    test_assert(as_bool(changed));

    // Subsequent call should return false
    evaluate_term(changed);
    test_assert(!as_bool(changed));
    evaluate_term(changed);
    test_assert(!as_bool(changed));

    // Change the modified time
    files.last_modified("x")++;
    evaluate_term(changed);
    test_assert(as_bool(changed));
    evaluate_term(changed);
    test_assert(!as_bool(changed));

    // Change the filename
    set_str(filename, "y");
    evaluate_term(changed);
    test_assert(as_bool(changed));
    evaluate_term(changed);
    test_assert(!as_bool(changed));
}

void test_include_namespace()
{
    Branch branch;
    FakeFileSystem files;
    files["file"] = "namespace ns a = 5 end";

    branch.eval("include('file')");
    Term* a = branch.eval("ns:a");

    test_assert(branch);
    test_assert(as_int(a) == 5);
}

void test_include_with_error()
{
    // If we include a file, and this file has a static error, then make sure
    // that this error is not hidden. In previous code, the include() function
    // would reject the errorred code, and then we'd never know an error happened.

    Branch branch;
    FakeFileSystem files;
    files["file"] = "eyjafjallajokull";

    branch.eval("include('file')");
    test_assert(has_static_errors(branch));
}

void register_tests()
{
    REGISTER_TEST_CASE(file_based_tests::test_the_test);
    REGISTER_TEST_CASE(file_based_tests::test_include_function);
    REGISTER_TEST_CASE(file_based_tests::test_include_static_error_after_reload);
    REGISTER_TEST_CASE(file_based_tests::test_file_changed);
    REGISTER_TEST_CASE(file_based_tests::test_include_namespace);
    REGISTER_TEST_CASE(file_based_tests::test_include_with_error);
}

} // namespace file_based_tests

} // namespace circa
