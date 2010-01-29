// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace term_to_source_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Term* term = caller->input(0);
        set_str(caller, get_term_source(term));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "term_to_source(any) -> string");
        function_t::set_input_meta(main_func, 0, true);
    }
}
}
