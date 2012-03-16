// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "filesystem.h"
#include "string_type.h"
#include "term.h"

namespace circa {

StorageInterface g_storageInterface;

void read_text_file_to_value(const char* filename, caValue* contents, caValue* error)
{
    struct ReceiveFile {
        caValue* _contents;
        caValue* _error;
        static void Func(void* context, const char* contents, const char* error) {
            ReceiveFile* obj = (ReceiveFile*) context;

            if (contents == NULL)
                set_string(obj->_contents, "");
            else
                set_string(obj->_contents, contents);

            if (obj->_error != NULL) {
                if (error == NULL)
                    set_null(obj->_error);
                else
                    set_string(obj->_error, error);
            }
        }
    };

    ReceiveFile obj;
    obj._contents = contents;
    obj._error = error;

    read_text_file(filename, ReceiveFile::Func, &obj);
}

std::string read_text_file_as_str(const char* filename)
{
    caValue contents;
    read_text_file_to_value(filename, &contents, NULL);

    if (is_string(&contents))
        return as_string(&contents);
    return "";
}

void read_directory(const char* dirname, ReadDirectoryCallback callback,
    void* context)
{
    if (g_storageInterface.readDirectory == NULL) {
        callback(context, NULL, "No storage interface is active");
        return;
    }
    g_storageInterface.readDirectory(dirname, callback, context);
}

bool read_directory_as_list_callback(void* context, const char* filename, const char* error)
{
    List* list = (List*) context;
    if (filename != NULL)
        set_string(list->append(), filename);
    return true;
}

void read_directory_as_list(const char* dirname, List* result)
{
    read_directory(dirname, read_directory_as_list_callback, (void*) result);
}

void install_storage_interface(StorageInterface* interface)
{
    g_storageInterface = *interface;
}

void get_current_storage_interface(StorageInterface* interface)
{
    *interface = g_storageInterface;
}

std::string get_directory_for_filename(std::string const& filename)
{
    // TODO: This function is terrible, need to use an existing library for dealing
    // with paths.
    size_t last_slash = filename.find_last_of("/");

    if (last_slash == filename.npos)
        return ".";

    if (last_slash == 0)
        return "/";

    std::string result = filename.substr(0, last_slash);

    return result;
}

bool is_absolute_path(std::string const& path)
{
    // TODO: This function is terrible, need to use an existing library for dealing
    // with paths.
    
    if (path.length() >= 1 && path[0] == '/')
        return true;
    if (path.length() >= 2 && path[1] == ':')
        return true;
    return false;
}

std::string get_absolute_path(std::string const& path)
{
    if (is_absolute_path(path))
        return path;

#if !CIRCA_ENABLE_FILESYSTEM
    return path;
#endif

    char buf[512];

#ifdef WINDOWS
    std::string cwd = _getcwd(buf, 512);
#else
    std::string cwd = getcwd(buf, 512);
#endif

    return cwd + "/" + path;
}

void read_text_file(const char* filename, ReadFileCallback callback, void* context)
{
    if (g_storageInterface.readTextFile == NULL)
        return callback(context, NULL, "No storage interface is active");
    g_storageInterface.readTextFile(filename, callback, context);
}

void write_text_file(const char* filename, const char* contents)
{
    if (g_storageInterface.writeTextFile == NULL)
        return;
    return g_storageInterface.writeTextFile(filename, contents);
}

time_t get_modified_time(const char* filename)
{
    if (filename[0] == 0)
        return 0;

    if (g_storageInterface.getModifiedTime == NULL)
        return 0;

    return g_storageInterface.getModifiedTime(filename);
}

bool file_exists(const char* filename)
{
    if (g_storageInterface.fileExists == NULL)
        return false;
    return g_storageInterface.fileExists(filename);
}

bool is_path_seperator(char c)
{
    // Not UTF safe.
    return c == '/' || c == '\\';
}

void join_path(String* left, String* right)
{
    const char* leftStr = as_cstring(left);
    const char* rightStr = as_cstring(right);
    int left_len = strlen(leftStr);
    int right_len = strlen(leftStr);

    int seperatorCount = 0;
    if (left_len > 0 && is_path_seperator(leftStr[left_len-1]))
        seperatorCount++;

    if (right_len > 0 && is_path_seperator(rightStr[0]))
        seperatorCount++;

    if (seperatorCount == 2)
        string_resize(left, left_len - 1);
    else if (seperatorCount == 0)
        string_append(left, "/");

    string_append(left, right);
}

} // namespace circa

// defined in filesystem_posix.cpp:
void install_posix_filesystem_interface();

// defined in file_posix.cpp
EXPORT void install_posix_file_source();

EXPORT void circa_use_standard_filesystem()
{
    // Old version
    install_posix_filesystem_interface();
    
    // New version
    install_posix_file_source();
}
