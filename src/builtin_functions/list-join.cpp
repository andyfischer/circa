// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace list_join_function {

    void evaluate(Term* caller)
    {
        recycle_value(caller->input(0), caller);

        List& result = as<List>(caller);
        List& second = as<List>(caller->input(1));

        for (int i=0; i < second.count(); i++) {
            result.append(second[i]);
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function list-join(List, List) -> List");
        as_function(main_func).pureFunction = true;
    }
}
}
