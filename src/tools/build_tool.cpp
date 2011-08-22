// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "evaluation.h"
#include "names.h"
#include "tagged_value.h"
#include "types/list.h"

namespace circa {

int run_build_tool(const char* filename)
{
    List buildArgs;
    set_string(buildArgs.append(), filename);

    TaggedValue* result = evaluate(get_global("cppbuild:build_module"), &buildArgs);

    if (is_error(result)) {
        std::cout << as_cstring(result) << std::endl;
        return -1;
    }

    return 0;
}

} // namespace circa
