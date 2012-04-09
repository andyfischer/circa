// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "circa/file.h"

#include "list.h"
#include "tagged_value.h"
#include "type.h"

using namespace circa;

extern "C" {

caValue* g_fileServices = NULL;
Type* g_fileServiceType = NULL;

std::map<std::string, caFileRecord*> g_fileRecords;

// Initialize globals (if needed)
static void file_init_globals()
{
    if (g_fileServices != NULL)
        return;

    g_fileServices = circa_alloc_value();
    set_list(g_fileServices, 0);

    g_fileServiceType = create_type();
    g_fileServiceType->name = name_from_string("FileService");
}

// Find the installed index of the given source name. Higher numbers take precedence.
// Returns -1 if no source with that name is installed.
static int find_index_of_source(Name sourceName)
{
    for (int i=0; i < list_length(g_fileServices); i++) {
        caFileSource* source = (caFileSource*) get_pointer(list_get(g_fileServices, i));
        if (source->name == sourceName)
            return i;
    }
    return -1;
}

void circa_install_file_source(caFileSource* source)
{
    file_init_globals();

    caValue* entry = list_append(g_fileServices);

    caFileSource* serviceCopy = (caFileSource*) malloc(sizeof(caFileSource));
    memcpy(serviceCopy, source, sizeof(caFileSource));

    set_pointer(entry, g_fileServiceType, serviceCopy);
}

static caFileSource* get_source_by_precedence(int precedence)
{
    file_init_globals();

    int index = list_length(g_fileServices) - precedence - 1;
    if (index < 0)
        return NULL;
    return (caFileSource*) get_pointer(list_get(g_fileServices, index));
}

static caFileSource* get_source_by_name(caName name)
{
    file_init_globals();

    for (int i=0;; i++) {
        caFileSource* source = get_source_by_precedence(i);
        if (source->name == name)
            return source;
    }
    return NULL;
}

caFileRecord* circa_fetch_file_record(const char* filename, caName source)
{
    file_init_globals();

    std::map<std::string, caFileRecord*>::const_iterator it;

    it = g_fileRecords.find(filename);

    if (it == g_fileRecords.end()) {

        // Record does not exist; create it.
        caFileRecord* record = (caFileRecord*) malloc(sizeof(caFileRecord));
        record->version = 1;
        record->source = source;
        record->data = NULL;
        record->filename = strdup(filename);
        record->sourceMetadata = circa_alloc_value();

        g_fileRecords[filename] = record;
        return record;

    } else {
        caFileRecord* record = it->second;

        // Record already exists.

        // Check if the existing record takes precedence
        int existingPrecedence = find_index_of_source(record->source);
        int callerPrecedence = find_index_of_source(source);

        if (existingPrecedence > callerPrecedence)
            return NULL;

        // Check if the existing record is for the same source.
        if (record->source == source)
            return record;

        // Existing record is for a different source; change its ownership.
        circa_set_null(record->sourceMetadata);
        record->source = source;
        return record;
    }
}

caFileRecord* circa_get_file_record(const char* filename)
{
    file_init_globals();

    std::map<std::string, caFileRecord*>::const_iterator it;
    it = g_fileRecords.find(filename);

    if (it != g_fileRecords.end())
        return it->second;

    return NULL;
}

static caFileRecord* circa_open_file(const char* filename)
{
    file_init_globals();

    caFileRecord* record = circa_get_file_record(filename);

    if (record == NULL) {
        // Record not found. Ask every file source if it can load this new file.
        for (int i=0;; i++) {
            caFileSource* source = get_source_by_precedence(i);
            if (source == NULL)
                break;

            if (source->openFile != NULL) {
                record = source->openFile(source, filename);
                if (record != NULL)
                    return record;
            }
        }

        // File not found
        return NULL;
    }

    caFileSource* source = get_source_by_name(record->source);

    // Call the source's updateFile handler (optional)
    if (source->updateFile != NULL)
        source->updateFile(source, record);
    
    return record;
}

const char* circa_read_file(const char* filename)
{
    caFileRecord* record = circa_open_file(filename);
    if (record == NULL)
        return NULL;
    return record->data;
}

bool circa_file_exists(const char* filename)
{
    caFileRecord* record = circa_open_file(filename);
    return record != NULL;
}

int circa_file_get_version(const char* filename)
{
    caFileRecord* record = circa_open_file(filename);
    if (record == NULL)
        return -1;
    return record->version;
}

} // extern "C"
