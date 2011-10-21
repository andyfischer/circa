// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

#include "branch.h"
#include "file_checker.h"
#include "source_repro.h"
#include "static_checking.h"
#include "filesystem.h"
#include "tagged_value.h"
#include "types/list.h"

namespace circa {

int run_file_checker_for_directory(const char* dir);

void run_file_checker(const char* filename, List* errors)
{
    Branch branch;
    load_script(&branch, filename);

    // Catch static errors
    {
        List staticErrors;
        check_for_static_errors(&staticErrors, &branch);
        for (int i=0; i < staticErrors.length(); i++)
            format_static_error(staticErrors[i], errors->append());
    }

    // Fetch the file as a string
    std::string actualSource;
    {
        TaggedValue fileContents;
        TaggedValue fileReadError;
        read_text_file_to_value(filename, &fileContents, &fileReadError);
        if (!is_null(&fileReadError)) {
            std::stringstream msg;
            msg << "File read error: " << fileReadError.toString();
            errors->appendString(msg.str());
            return;
        }
        actualSource = as_cstring(&fileContents);
    }

    // Run a source-repro test
    std::string reproducedSource = get_branch_source_text(&branch);

    if (actualSource != reproducedSource) {
        // TODO: Provide more details about the source repro problem.
        errors->appendString("Source reproduction failed");
    }
}

int run_file_checker(const char* filename)
{
    // check if this is a directory
    struct stat st_buf;
    int status = stat(filename, &st_buf);
    if ((status == 0) && S_ISDIR(st_buf.st_mode))
        return run_file_checker_for_directory(filename);

    List errors;
    run_file_checker(filename, &errors);
    if (is_null(&errors) || errors.empty())
        return 0;

    std::cout << filename << " had " << errors.length() << " error(s):\n";

    for (int i=0; i < errors.length(); i++)
        std::cout << as_cstring(errors[i]) << std::endl;

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
