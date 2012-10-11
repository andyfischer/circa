// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <sys/stat.h>
#include "circa/file.h"

#include "fakefs.h"
#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

using namespace circa;

extern "C" {

struct CachedFile {
    char* filename;
    circa::Value contents;
    bool needs_fread;
    int version;
    int last_known_mtime;
};

std::map<std::string, CachedFile*> g_fileCache;

static CachedFile* get_file_entry(const char* filename)
{
    std::map<std::string, CachedFile*>::const_iterator it;
    it = g_fileCache.find(filename);
    if (it == g_fileCache.end())
        return NULL;

    return it->second;
}

static CachedFile* create_file_entry(const char* filename)
{
    CachedFile* entry = get_file_entry(filename);
    if (entry != NULL)
        return entry;

    // Create a new entry
    entry = (CachedFile*) malloc(sizeof(*entry));
    entry->filename = circa_strdup(filename);
    initialize_null(&entry->contents);
    entry->needs_fread = false;
    entry->version = 0;
    entry->last_known_mtime = 0;
    g_fileCache[filename] = entry;
    return entry;
}

static void update_version_from_mtime(CachedFile* entry)
{
    unsigned mtime = 0;

    if (fakefs_enabled()) {
        mtime = fakefs_get_mtime(entry->filename);
    } else {
        struct stat s;
        s.st_mtime = 0;

        ca_assert(entry != NULL);
        ca_assert(entry->filename != NULL);

        stat(entry->filename, &s);
    }

    if (entry->last_known_mtime != mtime) {
        entry->last_known_mtime = mtime;
        entry->version++;
        entry->needs_fread = true;
    }
}

void circa_read_file(const char* filename, caValue* contentsOut)
{
    if (fakefs_enabled())
        return fakefs_read_file(filename, contentsOut);
    
    CachedFile* entry = create_file_entry(filename);
    update_version_from_mtime(entry);

    if (!entry->needs_fread)
        copy(&entry->contents, contentsOut);

    // Read the data
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        set_null(contentsOut);
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    char* contentsData = string_initialize(&entry->contents, file_size + 1);
    size_t bytesRead = fread(contentsData, 1, file_size, fp);

    contentsData[bytesRead] = 0;
    entry->needs_fread = false;

    copy(&entry->contents, contentsOut);

    log_start(0, "read_file");
    log_arg("filename", filename);
    log_arg("file_size", file_size);
    log_arg("bytesRead", bytesRead);
    log_arg("contents", entry->contents);
    log_finish();
}

bool circa_file_exists(const char* filename)
{
    if (fakefs_enabled())
        return fakefs_file_exists(filename);

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return false;

    fclose(fp);
    return true;
}

int circa_file_get_version(const char* filename)
{
    CachedFile* entry = create_file_entry(filename);
    update_version_from_mtime(entry);
    return entry->version;
}

} // extern "C"
