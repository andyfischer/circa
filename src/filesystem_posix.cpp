// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifdef WINDOWS
// for fopen()
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <fstream>
#include <sys/stat.h>

#include "filesystem.h"

static void read_text_file(const char* filename, circa::FileReceiveFunc receiveFile, void* context)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return receiveFile(context, NULL, "couldn't read file");

    // get file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char* buffer = (char*) malloc(size + 1);

    size_t bytes_read = fread(buffer, 1, size, fp);
    if (bytes_read != size)
        return receiveFile(context, NULL, "failed to read entire file");

    buffer[size] = 0;

    receiveFile(context, buffer, NULL);

    free(buffer);
    fclose(fp);
}

static void write_text_file(const char* filename, const char* contents)
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    file << contents;
    file.close();
}

static time_t get_modified_time(const char* filename)
{
    struct stat s;
    s.st_mtime = 0;

    stat(filename, &s);

    return s.st_mtime;
}

static bool file_exists(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp) {
        // file exists
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

void install_posix_filesystem_interface()
{
    circa::StorageInterface interface;
    interface.readTextFile = read_text_file;
    interface.writeTextFile = write_text_file;
    interface.getModifiedTime = get_modified_time;
    interface.fileExists = file_exists;
    circa::install_storage_interface(&interface);
}
