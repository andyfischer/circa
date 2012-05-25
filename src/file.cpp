// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <sys/stat.h>
#include "circa/file.h"

#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

using namespace circa;

extern "C" {

struct CachedFile {
    char* filename;
    char* contents;
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
    entry->contents = NULL;
    entry->needs_fread = false;
    entry->version = 0;
    entry->last_known_mtime = 0;
    g_fileCache[filename] = entry;
    return entry;
}

static void update_version_from_mtime(CachedFile* entry)
{
    struct stat s;
    s.st_mtime = 0;

    ca_assert(entry != NULL);
    ca_assert(entry->filename != NULL);

    stat(entry->filename, &s);

    if (entry->last_known_mtime != s.st_mtime) {
        entry->last_known_mtime = s.st_mtime;
        entry->version++;
        entry->needs_fread = true;
    }
}

const char* circa_read_file(const char* filename)
{
    CachedFile* entry = create_file_entry(filename);
    update_version_from_mtime(entry);

    if (!entry->needs_fread)
        return entry->contents;

    // Read the data
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);

    rewind(fp);

    entry->contents = (char*) realloc(entry->contents, file_size + 1);
    size_t bytesRead = fread(entry->contents, 1, file_size, fp);
    
    if (bytesRead != file_size)
        printf("failed to read entire file: %s", filename);

    entry->contents[file_size] = 0;
    entry->needs_fread = false;

    return entry->contents;
}

bool circa_file_exists(const char* filename)
{
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
