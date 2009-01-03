// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace term_to_source_line_function {

    void evaluate(Term* caller)
    {
        Term* term = caller->input(0);

        std::stringstream result;

        if (term->name != "") {
            result << term->name << " = ";
        }

        result << term->function->name << "(";

        for (int i=0; i < term->numInputs(); i++) {
            Term* input = term->input(i);

            if (i > 0) result << ", ";

            result << input->name;
        }

        result << ")";

        as_string(caller) = result.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function term-to-source-line(any) -> string");
        as_function(main_func).pureFunction = true;
        as_function(main_func).setInputMeta(0, true);
    }
}
}
