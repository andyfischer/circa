// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace include_function {

    void evaluate(Term* caller)
    {
        std::string filename = caller->input(0)->asString();

        filename = get_source_file_location(*caller->owningBranch) + "/" + filename;

        Branch& result = as_branch(caller);

        Term* existing_source_file = get_branch_attribute(result, "source-file");

        if (existing_source_file == NULL || (filename != existing_source_file->asString())) {
            result.clear();
            parse_script(result, filename);
        }

        assert(get_branch_attribute(result, "source-file")->asString() == filename);

        evaluate_branch(as_branch(caller));
    }

    std::string toSourceString(Term* term)
    {
        return "include " + term->input(0)->asString();
    }

    void setup(Branch& kernel)
    {
        INCLUDE_FUNC = import_function(kernel, evaluate, "include(string filename) : Branch");
        function_t::get_to_source_string(INCLUDE_FUNC) = toSourceString;
    }
}
}
