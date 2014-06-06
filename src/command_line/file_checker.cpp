// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "kernel.h"
#include "list.h"
#include "static_checking.h"
#include "tagged_value.h"

#include "file_checker.h"

namespace circa {

int run_file_checker_for_directory(const char* dir);

void run_file_checker(const char* filename, Value* errors)
{
    Block block;
    load_script(&block, filename);

    // Catch static errors
    {
        Value staticErrors;
        set_list(&staticErrors, 0);
        check_for_static_errors(&staticErrors, &block);
        for (int i=0; i < staticErrors.length(); i++)
            format_static_error(staticErrors.index(i), errors->append());
    }

    // Fetch the file as a string
    std::string actualSource;
    {
        Value fileContents;
        Value fileReadError;

        circa_read_file(global_world(), filename, &fileContents);
        if (is_null(&fileContents)) {
            std::stringstream msg;
            msg << "File not found: " << filename;
            set_string(errors->append(), msg.str());
            return;
        }
        actualSource = as_cstring(&fileContents);
    }
}

int run_file_checker(const char* filename)
{
    // check if this is a directory
    struct stat st_buf;
    int status = stat(filename, &st_buf);
    if ((status == 0) && S_ISDIR(st_buf.st_mode))
        return run_file_checker_for_directory(filename);

    Value errors;
    set_list(&errors, 0);
    run_file_checker(filename, &errors);
    if (is_null(&errors) || errors.isEmpty())
        return 0;

    std::cout << filename << " had " << errors.length() << " error(s):\n";

    for (int i=0; i < errors.length(); i++)
        std::cout << as_cstring(errors.index(i)) << std::endl;

    return -1;
}

int run_file_checker_for_directory(const char* filename)
{
    DIR* dir = opendir(filename);
    if (dir == NULL) {
        std::cout << "failed to open directory: " << filename << std::endl;
        return -1;
    }

    int finalResult = 0;
    while (true) {
        struct dirent *ent = readdir(dir);
        if (ent == NULL)
            break;

        std::string path(ent->d_name);

        if (path == "." || path == "..")
            continue;

        path = std::string(filename) + "/" + path;

        // recurse on directories
        struct stat st_buf;
        int status = stat(path.c_str(), &st_buf);
        if ((status == 0) && S_ISDIR(st_buf.st_mode)) {
            int result = run_file_checker_for_directory(path.c_str());
            if (result != 0)
                finalResult = -1;
            continue;
        }

        // skip if it doesn't end in .ca
        if (path.rfind(".ca") != (path.length() - 3)) {
            continue;
        }

        int result = run_file_checker(path.c_str());
        if (result != 0)
            finalResult = -1;
        else
            std::cout << "Passed: " << path << std::endl;
    }
    closedir(dir);
    return finalResult;
}

} // namespace circa
