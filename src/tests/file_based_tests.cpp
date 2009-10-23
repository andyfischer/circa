// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_based_tests {

void test_the_test()
{
    // Make sure that the file IO redirection thingy is working.

    FakeFileSystem files;

    files["a.txt"] = "hello";
    test_assert(read_text_file("a.txt") == "hello");

    write_text_file("a.txt", "goodbye");
    test_assert(read_text_file("a.txt") == "goodbye");
    test_assert(files["a.txt"] == "goodbye");
}

void test_include_function()
{
    FakeFileSystem files;

    files["file.ca"] = "a = 1";

    // Basic test, load our fake file.
    Branch branch;
    Term* incl = branch.eval("include('file.ca')");

    test_assert(is_branch(incl));
    test_assert(incl->asBranch()["a"]->asInt() == 1);
    test_equals(incl->asBranch()["#attr:source-file"]->asString(), "file.ca");

    // Next, modify the file and reload.
    files["file.ca"] = "b = 2";
    files.last_modified("file.ca")++;

    evaluate_term(incl);

    test_assert(!incl->asBranch().contains("a"));
    test_assert(incl->asBranch()["b"]->asInt() == 2);

    // Modify the file but don't modify the last_modified time, make sure that
    // it doesn't reload this time.
    files["file.ca"] = "c = 3";
    evaluate_term(incl);
    test_assert(!incl->asBranch().contains("a"));
    test_assert(!incl->asBranch().contains("c"));
    test_assert(incl->asBranch()["b"]->asInt() == 2);

    // Modify the file with a script that has errors
    files["file.ca"] = "add(what what)";
    files.last_modified("file.ca")++;

    evaluate_term(incl);

    test_assert(has_static_errors(incl->asBranch()));
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
    as_string(filename) = "y";
    evaluate_term(changed);
    test_assert(as_bool(changed));
    evaluate_term(changed);
    test_assert(!as_bool(changed));
}

void register_tests()
{
    REGISTER_TEST_CASE(file_based_tests::test_the_test);
    REGISTER_TEST_CASE(file_based_tests::test_include_function);
    REGISTER_TEST_CASE(file_based_tests::test_file_changed);
}

} // namespace file_based_tests

} // namespace circa
