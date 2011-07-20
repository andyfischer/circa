// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "importing.h"
#include "importing_macros.h"
#include "storage.h"
#include "stdlib.h"

namespace circa {

CA_FUNCTION(file__modified_time)
{
    set_int(OUTPUT, storage::get_modified_time(STRING_INPUT(0)));
}

void install_stdlib_functions(Branch& kernel)
{
    install_function(kernel["file:modified_time"], file__modified_time);
}

} // namespace circa
