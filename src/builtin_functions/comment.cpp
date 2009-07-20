// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace comment_function {

    void evaluate(Term* caller)
    {
    }

    std::string toSourceString(Term* term)
    {
        return get_comment_string(term);
    }

    void setup(Branch& kernel)
    {
        COMMENT_FUNC = import_function(kernel, evaluate, "comment()");
        function_t::get_to_source_string(COMMENT_FUNC) = toSourceString;
    }
}
}
