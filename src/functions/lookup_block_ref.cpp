// Copyright (c) 2007-2010 Andrew Fischer. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace lookup_block_ref_function {

    CA_FUNCTION(evaluate)
    {
        #if 0
        std::string name = as_string(INPUT(0));
        Term* term = get_global(name);

        if (term == NULL)
            return block_ref_t::set_from_ref(OUTPUT, NULL);

        // FIXME: Don't give references to a non-block

        return block_ref_t::set_from_ref(OUTPUT, term);
        #endif
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate, "lookup_block_ref(String) -> Block");
    }
}
}
