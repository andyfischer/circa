// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/** fakefs.h
 *
 * Fake filesystem. When enabled, all file-reading commands will be redirected here.
 * Used primarily for testing.
 *
 */

#pragma once

namespace circa {

// Whether the fake filesystem is currently enabled.
bool fakefs_enabled();

void enable_fakefs();
void disable_fakefs();

void fakefs_read_file(const char* filename, caValue* contentsOut);
bool fakefs_file_exists(const char* filename);
int fakefs_get_mtime(const char* filename);

// Helper class to enable/disable the fake filesystem in a scope.
struct FakeFilesystem
{
    FakeFilesystem();
    ~FakeFilesystem();

    void set(std::string filename, std::string data);
    void set_mtime(std::string filename, int time);
};

} // namespace circa
