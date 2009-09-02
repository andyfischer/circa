// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

// Not sure if this function is a good idea, it might get removed.

namespace circa {
namespace eval_script_function {

    void evaluate(Term* caller)
    {
        std::string filename = caller->input(0)->asString();
        Branch& result = as_branch(caller);

        if (filename != get_branch_source_filename(result)) {
            result.clear();
            parse_script(result, filename);
        }

        assert(get_branch_source_filename(result) == filename);

        evaluate_branch(result);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "eval_script(string filename) : Branch");
    }
}
}
