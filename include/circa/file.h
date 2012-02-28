// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_FILE_H_INCLUDED
#define CIRCA_FILE_H_INCLUDED

#include "circa.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

// A "file source" is an object that acts as a file system. It supports file reading
// and writing. One examlpe of a file source is one that uses the current computer's
// filesystem (and Circa includes a standard implementation of this). Other file sources
// could be fully in-memory, or use sockets to fetch files from a remote host.
//
// Circa supports multiple active file sources at once. The most-recently-loaded one
// takes precedence. When loading a file we'll first check the most-recent source, and
// if it doesn't know the file, then we'll proceed to ask the next source.

typedef struct caFileSource caFileSource;
typedef struct caFileRecord caFileRecord;

typedef caFileRecord* (*caFileSourceOpenFile) (caFileSource* source, const char* filename);
typedef void (*caFileSourceUpdateFile) (caFileSource* source, caFileRecord* record);

typedef struct caFileSource {

    // openFile is called when we are trying to open a file that has no existing record.
    // If the file source successfully opens the file, it should create a new FileRecord
    // and return it. Otherwise it can return NULL.
    //
    // This function is optional.
    caFileSourceOpenFile openFile;

    // updateFile is called immediately before we access the data for a given file.
    // Depending on the source, it may update the FileRecord.data field. If the file
    // source chooses to update FileSource.data on its own time, then it doesn't
    // need to do anything here.
    //
    // This function is optional
    caFileSourceUpdateFile updateFile;

    caName name;

} caFileSource;

// Install a file source. This function will make its own copy of 'source'.
void circa_install_file_source(caFileSource* source);

// There is one "file record" object for every file that is loaded. The record stores
// the file's current version (which callers can keep track of if they are interested
// in file changes). The record also stores an identifier for the source that
// provided this file.
typedef struct caFileRecord {
    const char* filename;
    int version;

    caName source;

    caValue* sourceMetadata;
    caMutex* mutex;
    char* data;

} caFileRecord;

// Fetch the file record for this filename. If the record doesn't exist then we'll
// create a new one (initialized with the given source). If the record already exists,
// then we may update the record to use the provided source, if the source takes
// precedence over the existing one.
//
// If the provided source doesn't take precedence over the one that the record is
// currently using, then this function returns NULL.
//
caFileRecord* circa_fetch_file_record(const char* filename, caName source);

// Find the file record with the given filename, returns NULL if the record does not
// currently exist.
caFileRecord* circa_get_file_record(const char* filename);

const char* circa_read_file(const char* filename);

int circa_file_get_version(const char* filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
