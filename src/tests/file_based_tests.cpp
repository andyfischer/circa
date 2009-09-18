// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace file_based_tests {

struct FakeFileSystem : FileIORedirector
{
    struct File {
        std::string contents;
        time_t last_modified;

        File() : last_modified(0) {}
    };

    std::map<std::string, File> _files;

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

} FakeFileSystem;

struct FakeFileSystemInstaller
{
    FakeFileSystemInstaller() { install_fake_file_io(&FakeFileSystem); }
    ~FakeFileSystemInstaller() { install_fake_file_io(NULL); }
};

void test_the_test()
{
    // Make sure that the file IO redirection thingy is working.

    FakeFileSystemInstaller installer;

    FakeFileSystem["a.txt"] = "hello";
    test_assert(read_text_file("a.txt") == "hello");

    write_text_file("a.txt", "goodbye");
    test_assert(read_text_file("a.txt") == "goodbye");
    test_assert(FakeFileSystem["a.txt"] == "goodbye");
}

void register_tests()
{
    REGISTER_TEST_CASE(file_based_tests::test_the_test);
}

} // namespace file_based_tests

} // namespace circa
