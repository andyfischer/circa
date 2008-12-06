// Copyright 2008 Paul Hodge

#include "circa.h"
#include "list.h"
#include "symbolic_list.h"

namespace circa {
namespace slist_pack_function {

    void evaluate(Term* caller) {

        SymbolicRefList &result = as<SymbolicRefList>(caller);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function slist-pack(any) -> SList");
        as_function(main_func).variableArgs = true;
        as_function(main_func).pureFunction = true;
    }
}
}
