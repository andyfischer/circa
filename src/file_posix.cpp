// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "circa/file.h"

extern "C" {

static void update_file(caFileSource*, caFileRecord* record)
{
    FILE* fp = fopen(record->filename, "r");
    if (fp == NULL) {
        // File no longer exists
        free(record->data);
        record->data = NULL;
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    record->data = (char*) realloc(record->data, size + 1);
    fread(record->data, 1, size, fp);
    record->data[size] = 0;

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
