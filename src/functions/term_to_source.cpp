// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace term_to_source_function {

    CA_FUNCTION(evaluate)
    {
        Term* term = INPUT_TERM(0);
        set_string(OUTPUT, get_term_source_text(term));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "term_to_source(any) -> string");
        function_t::set_input_meta(main_func, 0, true);
    }
}
}
