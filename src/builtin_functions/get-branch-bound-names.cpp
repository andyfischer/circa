// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "builtins.h"
#include "circa.h"
#include "set.h"
#include "term_namespace.h"

namespace circa {
namespace get_branch_bound_names_function {

    void evaluate(Term* caller)
    {
        Branch &branch = as_branch(caller->input(0));
        Branch &result = as_branch(caller);
        result.clear();

        TermNamespace::iterator it;
        for (it = branch.names._map.begin(); it != branch.names._map.end(); ++it) {
            std::string const& name = it->first;

            Term* value = create_value(&result, STRING_TYPE);
            as_string(value) = name;
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function get-branch-bound-names(Branch) -> Branch");
        as_function(main_func).pureFunction = true;
    }
}
}
