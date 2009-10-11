// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_FILE_IO_INCLUDED
#define CIRCA_FILE_IO_INCLUDED

// Miscellaneous functions for reading to disk.

namespace circa {

std::string read_text_file(std::string const& filename);
void write_text_file(std::string const& filename, std::string const& contents);
time_t get_modified_time(std::string const& filename);
bool file_exists(std::string const& filename);

std::string get_path_relative_to_source(Term* relativeTo, std::string const& path);
std::string get_directory_for_filename(std::string const& filename);

// To redirect any IO calls, implement this interface and then call install_fake_file_io().
// This should only be used for testing. Make sure to clean up the redirector when you are done
// with it (call install_fake_file_io with NULL).
struct FileIORedirector {
    virtual ~FileIORedirector() {}
    virtual std::string read_text_file(std::string const& filename) = 0;
    virtual void write_text_file(std::string const& filename, std::string const& contents) = 0;
    virtual time_t get_modified_time(std::string const& filename) = 0;
    virtual bool file_exists(std::string const& filename) = 0;
};

// Call this with NULL to reset it.
void install_fake_file_io(FileIORedirector* fakeObject);

} // namespace circa

#endif
