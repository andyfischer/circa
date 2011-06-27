// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "types/set.h"

namespace circa {
namespace set_methods_function {

    CA_FUNCTION(hosted_add)
    {
        copy(INPUT(0), OUTPUT);
        List* output = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);
        if (!set_t::contains(output, value))
            copy(value, output->append());
    }

    CA_FUNCTION(contains)
    {
        List* list = List::checkCast(INPUT(0));
        TaggedValue* value = INPUT(1);
        set_bool(OUTPUT, set_t::contains(list, value));
    }

    CA_FUNCTION(remove)
    {
        copy(INPUT(0), OUTPUT);
        List* list = List::checkCast(OUTPUT);
        TaggedValue* value = INPUT(1);

        int numElements = list->numElements();
        for (int index=0; index < numElements; index++) {
            if (equals(value, list->get(index))) {
                list_t::remove_and_replace_with_back(list, index);
                return;
            }
        }
    }
    
    void setup(Branch& kernel)
    {
        import_function(kernel, hosted_add, "Set.add(self :implied_rebind, any) -> Set");
        import_function(kernel, remove, "Set.remove(self :implied_rebind, any) -> Set");
        import_function(kernel, contains, "Set.contains(self, any) -> bool");
    }

}
}
