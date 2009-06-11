// Copyright 2008 Andrew Fischer

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
        Term* main_func = import_function(kernel, evaluate, "comment()");
        as_function(main_func).toSourceString = toSourceString;

        COMMENT_FUNC = main_func;
    }
}
}
