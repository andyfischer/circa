// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "introspection.h"

namespace circa {
namespace term_to_source_function {

    void evaluate(Term* caller)
    {
        Term* term = caller->input(0);
        as_string(caller) = get_term_source(term);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "term_to_source(any) : string");
        function_set_input_meta(main_func, 0, true);
    }
}
}
