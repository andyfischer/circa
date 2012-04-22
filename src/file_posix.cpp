// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#if CIRCA_ENABLE_FILESYSTEM

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "common_headers.h"

#include "circa/file.h"

#include "names.h"

extern "C" {

static int get_modified_time2(const char* filename)
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
    int modifiedTime = get_modified_time2(record->filename);
    if (circa_is_int(record->sourceMetadata) &&
        modifiedTime == circa_int(record->sourceMetadata)) {

        goto finish;
    }

    // If we reach this point then we'll read the file

    // Store modified time
    circa_set_int(record->sourceMetadata, modifiedTime);

    {
	// Get file size
        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        rewind(fp);

        record->data = (char*) realloc(record->data, size + 1);
        fread(record->data, 1, size, fp);
        record->data[size] = 0;
        record->version++;
    }

finish:
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

} // extern "C"

extern "C" void circa_use_standard_filesystem(caWorld*)
{
    caFileSource source;
    memset(&source, 0, sizeof(source));
    source.openFile = open_file;
    source.updateFile = update_file;
    source.name = circa::name_from_string("builtin:PosixFileSource");

    circa_install_file_source(&source);
}

#else // CIRCA_ENABLE_FILESYSTEM

extern "C" void circa_use_standard_filesystem(caWorld*)
{
    internal_error("POSIX file source is unavailable, CIRCA_ENABLE_FILESYSTEM is off");
}

#endif
