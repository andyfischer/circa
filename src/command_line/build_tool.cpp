// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../evaluation.h"
#include "../names.h"
#include "../tagged_value.h"
#include "../list.h"

namespace circa {

int run_build_tool(caValue* args)
{
    for (int i=0; i < list_length(args); i++) {

        const char* filename = as_cstring(list_get(args, i));
    
        List buildArgs;
        set_string(buildArgs.append(), filename);

#if 0  // fixme
        caValue* result = evaluate(get_global("cppbuild:build_module"), &buildArgs);

        if (is_error(result)) {
            std::cout << as_cstring(result) << std::endl;
            return -1;
        }
#endif
    }

    return 0;
}

} // namespace circa
