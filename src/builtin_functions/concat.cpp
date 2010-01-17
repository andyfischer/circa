// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace concat_function {

    void evaluate(Term* caller)
    {
        std::stringstream out;
        for (int index=0; index < caller->inputs.length(); index++) {
            Term* t = caller->inputs[index];
            if (is_string(t))
                out << as_string(caller->inputs[index]);
            else
                out << to_string(t);
        }
        set_str(caller, out.str());
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "concat(any...) -> string;"
            "'Concatenate each input (converting to a string if necessary).' end");
    }
}
}
