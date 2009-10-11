// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_based_tests {

// Class that implements FileIORedirector. This class also installs itself on construction
// and deletes itself afterwards.
struct FakeFileSystem : FileIORedirector
{
    struct File {
        std::string contents;
        time_t last_modified;

        File() : last_modified(0) {}
    };

    std::map<std::string, File> _files;

    FakeFileSystem() { install_fake_file_io(this); }
    ~FakeFileSystem() { install_fake_file_io(NULL); }

    std::string& operator [] (std::string const& filename)
    {
        return _files[filename].contents;
    }

    time_t& last_modified(std::string const& filename)
    {
        return _files[filename].last_modified;
    }

    virtual std::string read_text_file(std::string const& filename)
    {
        return _files[filename].contents;
    }

    virtual void write_text_file(std::string const& filename, std::string const& contents)
    {
        _files[filename].contents = contents;
    }

    virtual time_t get_modified_time(std::string const& filename)
    {
        return _files[filename].last_modified;
    }

    virtual bool file_exists(std::string const& filename)
    {
        return _files.find(filename) != _files.end();
    }
};

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

void register_tests()
{
    REGISTER_TEST_CASE(file_based_tests::test_the_test);
    REGISTER_TEST_CASE(file_based_tests::test_include_function);
}

} // namespace file_based_tests

} // namespace circa
