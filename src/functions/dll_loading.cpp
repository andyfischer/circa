// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "code_iterators.h"
#include "dll_loading.h"
#include "function.h"
#include "importing.h"

namespace circa {
namespace dll_loading_function {

    CA_FUNCTION(load_and_patch)
    {
        Branch* branch = as_branch(INPUT(0));
        const char* filename = STRING_INPUT(1);

        TaggedValue error;
        patch_with_dll(filename, branch, &error);

        if (!is_null(&error))
            ERROR_OCCURRED(as_cstring(&error));
    }

    CA_FUNCTION(dll_filename)
    {
        std::string base_filename = STRING_INPUT(0);
        set_string(OUTPUT, base_filename + ".so");
    }
    CA_FUNCTION(source_filename)
    {
        std::string base_filename = STRING_INPUT(0);
        set_string(OUTPUT, base_filename + ".cpp");
    }

    void setup(Branch* kernel)
    {
        Branch* ns = create_namespace(kernel, "dll_loading");
        import_function(ns, load_and_patch,
                "load_and_patch(Branch branch, string filename)");
        import_function(ns, dll_filename,
                "dll_filename(string baseFilename) -> string");
        import_function(ns, source_filename,
                "source_filename(string baseFilename) -> string");
    }
}
} // namespace circa
