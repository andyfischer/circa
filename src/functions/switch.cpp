// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "switch_block.h"

namespace circa {
namespace switch_function {

    void formatSource(StyledSource* source, Term* term)
    {
        // TODO
    }

    void setup(Branch& kernel)
    {
        SWITCH_FUNC = import_function(kernel, evaluate_switch, "switch() -> any");
        get_function_attrs(SWITCH_FUNC)->formatSource = formatSource;
    }

} // namespace switch_function
} // namespace circa
