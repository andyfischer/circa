// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace type_producers_function {

    CA_FUNCTION(handle_type)
    {
        Type* type = create_type();

        set_type(OUTPUT, type);
    }

}
}
