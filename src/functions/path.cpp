// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>
#include "storage.h"

namespace circa {
namespace path_function {

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

    CA_DEFINE_FUNCTION(path, "def path(string) -> string")
    {
        make_string(OUTPUT,
            get_path_relative_to_source(CALLER, as_string(INPUT(0))));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
