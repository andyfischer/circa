// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#ifdef WINDOWS
// for fopen()
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "filesystem.h"

static void posix_read_text_file(const char* filename, circa::ReadFileCallback receiveFile, void* context)
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

static void posix_write_text_file(const char* filename, const char* contents)
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    file << contents;
    file.close();
}

static time_t posix_get_modified_time(const char* filename)
{
    struct stat s;
    s.st_mtime = 0;

    stat(filename, &s);

    return s.st_mtime;
}

static bool posix_file_exists(const char* filename)
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

static void posix_read_directory(const char* dirname, circa::ReadDirectoryCallback callback,
       void* context)
{
    DIR* dir = opendir(dirname);

    if (dir == NULL) {
        callback(context, NULL, "failed to open file as a directory");
        return;
    }

    while (true) {
        struct dirent *ent = readdir(dir);

        if (ent == NULL)
            break;

        // Don't expose the '.' or '..' entries
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
            continue;

        bool callbackContinue = callback(context, ent->d_name, NULL);
        
        if (!callbackContinue)
            break;
    }
    closedir(dir);
}

void install_posix_filesystem_interface()
{
    circa::StorageInterface interface;
    interface.readTextFile = posix_read_text_file;
    interface.writeTextFile = posix_write_text_file;
    interface.getModifiedTime = posix_get_modified_time;
    interface.fileExists = posix_file_exists;
    interface.readDirectory = posix_read_directory;
    circa::install_storage_interface(&interface);
}
