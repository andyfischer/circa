// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace list_remove_duplicates_function {

    void evaluate(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        List& list = as<List>(caller);

        for (int i=0; i < list.count(); i++) {
            Term* left_term = list[i];

            for (int j=i + 1; j < list.count(); j++) {
                Term* right_term = list[j];

                if (values_equal(left_term, right_term)) {
                    list.remove(j);
                    j--;
                }
            }
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function list-remove-duplicates(List) -> List");
        as_function(main_func).pureFunction = true;
    }
}
}
