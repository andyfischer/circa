// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/set.h"

namespace circa {
namespace set_methods_function {

    CA_FUNCTION(hosted_add)
    {
        copy(INPUT(0), EXTRA_OUTPUT(0));
        List* output = List::checkCast(EXTRA_OUTPUT(0));
        caValue* value = INPUT(1);
        if (!set_t::contains(output, value))
            copy(value, output->append());
    }

    CA_FUNCTION(contains)
    {
        List* list = List::checkCast(INPUT(0));
        caValue* value = INPUT(1);
        set_bool(OUTPUT, set_t::contains(list, value));
    }

    CA_FUNCTION(remove)
    {
        copy(INPUT(0), EXTRA_OUTPUT(0));
        List* list = List::checkCast(EXTRA_OUTPUT(0));
        caValue* value = INPUT(1);

        int numElements = list->numElements();
        for (int index=0; index < numElements; index++) {
            if (equals(value, list->get(index))) {
                list_remove_and_replace_with_last_element(list, index);
                return;
            }
        }
    }
    
    void setup(Branch* kernel)
    {
        import_function(kernel, hosted_add, "Set.add(self :out, any)");
        import_function(kernel, remove, "Set.remove(self :out, any)");
        import_function(kernel, contains, "Set.contains(self, any) -> bool");
    }

}
}
