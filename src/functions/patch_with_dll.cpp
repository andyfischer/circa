// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "dll_patching.h"

namespace circa {
namespace patch_with_dll_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(hosted_patch_with_dll,
            "patch_with_dll(state FileSignature, string filename)")
    {
        // TODO: check file signature

        const char* filename = STRING_INPUT(1);
        Value error;
        patch_with_dll(filename, *CALLER->owningBranch, &error);

        if (!is_null(&error))
            error_occurred(CONTEXT, CALLER, as_string(&error));
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
