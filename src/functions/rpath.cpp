// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "filesystem.h"

namespace circa {
namespace rpath_function {

    CA_START_FUNCTIONS;

    std::string get_path_relative_to_source(Term* relativeTo, std::string const& path)
    {
        // Don't modify a blank path
        if (path == "")
            return "";

        if (relativeTo->owningBranch == NULL)
            return path;

        // Don't modify absolute paths
        if (is_absolute_path(path))
            return path;

        std::string scriptLocation = get_source_file_location(*relativeTo->owningBranch);

        if (scriptLocation == "" || scriptLocation == ".")
            return path;

        return scriptLocation + "/" + path;
    }

    CA_DEFINE_FUNCTION(rpath, "def rpath(string) -> string")
    {
        set_string(OUTPUT,
            get_path_relative_to_source(CALLER, as_string(INPUT(0))));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
