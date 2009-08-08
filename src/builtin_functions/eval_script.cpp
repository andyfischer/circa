// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

// Not sure if this function is a good idea, it might get removed.

namespace circa {
namespace eval_script_function {

    void evaluate(Term* caller)
    {
        std::string filename = caller->input(0)->asString();
        Branch& result = as_branch(caller);

        Term* existing_source_file = get_branch_attribute(result, "source-file");

        if (existing_source_file == NULL || (filename != existing_source_file->asString())) {
            result.clear();
            parse_script(result, filename);
        }

        assert(get_branch_attribute(result, "source-file")->asString() == filename);

        evaluate_branch(result);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "eval_script(string filename) : Branch");
    }
}
}
