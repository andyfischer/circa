// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace static_error_function {

    void setup(Branch& kernel)
    {
        STATIC_ERROR_FUNC = 
            import_function(kernel, NULL, "static_error(any msg)");
    }

}
}
