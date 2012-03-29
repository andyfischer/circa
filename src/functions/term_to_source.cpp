// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace term_to_source_function {

    CA_FUNCTION(evaluate)
    {
        Term* term = INPUT_TERM(0);
        set_string(OUTPUT, get_term_source_text(term));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "term_to_source(any :meta) -> String");
    }
}
} // namespace circa
