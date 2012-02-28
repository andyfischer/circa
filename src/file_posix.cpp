// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "circa/file.h"

extern "C" {

static int get_modified_time(const char* filename)
{
    struct stat s;
    s.st_mtime = 0;

    stat(filename, &s);

    return s.st_mtime;
}

static void update_file(caFileSource*, caFileRecord* record)
{
    FILE* fp = fopen(record->filename, "r");
    if (fp == NULL) {
        // File no longer exists
        free(record->data);
        record->data = NULL;
        return;
    }

    // Check the last modified time to see if we need to reload the file.
    int modifiedTime = get_modified_time(record->filename);
    if (circa_is_int(record->sourceMetadata) &&
        modifiedTime == circa_as_int(record->sourceMetadata)) {

        return;
    }

    // If we reach this point then we'll read the file

    // Store modified time
    circa_set_int(record->sourceMetadata, modifiedTime);

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    record->data = (char*) realloc(record->data, size + 1);
    fread(record->data, 1, size, fp);
    record->data[size] = 0;
    record->version++;

    fclose(fp);
}

static caFileRecord* open_file(caFileSource* source, const char* filename)
{
    // Check if the file exists
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;

    fclose(fp);

    caFileRecord* record = circa_fetch_file_record(filename, source->name);
    update_file(source, record);
    return record;
}

void install_posix_file_source()
{
    caFileSource source;
    memset(&source, 0, sizeof(source));
    source.openFile = open_file;
    source.updateFile = update_file;
    source.name = circa_name("builtin:PosixFileSource");

    circa_install_file_source(&source);
}

} // extern "C"
