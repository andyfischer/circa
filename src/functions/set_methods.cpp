// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/set.h"

namespace circa {
namespace set_methods_function {

    void hosted_add(caStack* stack)
    {
        caValue* output = circa_output(stack, 1);
        copy(circa_input(stack, 0), output);

        caValue* value = circa_input(stack, 1);
        if (!set_t::contains(output, value))
            copy(value, list_append(output));
    }

    void contains(caStack* stack)
    {
        caValue* list = circa_input(stack, 0);
        caValue* value = circa_input(stack, 1);
        set_bool(circa_output(stack, 0), set_t::contains(list, value));
    }

    void remove(caStack* stack)
    {
        caValue* output = circa_output(stack, 1);
        copy(circa_input(stack, 0), output);

        caValue* value = circa_input(stack, 1);

        int numElements = list_length(output);
        for (int index=0; index < numElements; index++) {
            if (equals(value, list_get(output, index))) {
                list_remove_and_replace_with_last_element(output, index);
                return;
            }
        }
    }
    
    void setup(Block* kernel)
    {
        import_function(kernel, hosted_add, "Set.add(self :out, any)");
        import_function(kernel, remove, "Set.remove(self :out, any)");
        import_function(kernel, contains, "Set.contains(self, any) -> bool");
    }

}
}
