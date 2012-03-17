// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "filesystem.h"
#include "file_utils.h"
#include "string_type.h"
#include "term.h"

namespace circa {

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

} // namespace circa
