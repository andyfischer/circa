// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_identifier_function {

    void evaluate(Term* caller)
    {
    }

    std::string toSourceString(Term* term)
    {
        return term->name;
    }

    void setup(Branch& kernel)
    {
        UNKNOWN_IDENTIFIER_FUNC = import_function(kernel, evaluate, "unknown_identifier() :: any");
        function_t::get_to_source_string(UNKNOWN_IDENTIFIER_FUNC) = toSourceString;
    }
}
}
