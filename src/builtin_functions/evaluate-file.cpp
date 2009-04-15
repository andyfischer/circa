// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace evaluate_file_function {

    void evaluate(Term* caller)
    {
        std::string &filename = as_string(caller->input(0));
        Branch &branch = as_branch(caller);
        branch.clear();
        evaluate_file(branch, filename);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "evaluate_file(string) -> Branch");
    }
}
} // namespace circa
