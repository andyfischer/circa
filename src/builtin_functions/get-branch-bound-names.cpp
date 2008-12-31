// Copyright 2008 Paul Hodge

#include "branch.h"
#include "builtins.h"
#include "circa.h"
#include "set.h"
#include "term_namespace.h"

namespace circa {
namespace get_branch_bound_names_function {

    void evaluate(Term* caller)
    {
        Branch &branch = as<Branch>(caller->input(0));
        List &result = as<List>(caller);
        result.clear();

        TermNamespace::iterator it;
        for (it = branch.names._map.begin(); it != branch.names._map.end(); ++it) {
            std::string const& name = it->first;

            Term* value = create_value(NULL, STRING_TYPE);
            as_string(value) = name;

            result.append(value);

            delete value;
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function get-branch-bound-names(Branch) -> List");
        as_function(main_func).pureFunction = true;
    }
}
}
