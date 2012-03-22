// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../types/hashtable.h"

namespace circa {
namespace hashtable_methods_function {

    CA_FUNCTION(contains)
    {
        caValue* value = hashtable_t::get_value(INPUT(0), INPUT(1));
        set_bool(OUTPUT, value != NULL);
    }

    CA_FUNCTION(insert)
    {
        Value key, value;
        CONSUME_INPUT(0, EXTRA_OUTPUT(0));
        CONSUME_INPUT(1, &key);
        CONSUME_INPUT(2, &value);
        hashtable_t::table_insert(EXTRA_OUTPUT(0), &key, &value, true, true);
    }

    CA_FUNCTION(remove)
    {
        CONSUME_INPUT(0, EXTRA_OUTPUT(0));
        hashtable_t::table_remove(EXTRA_OUTPUT(0), INPUT(1));
    }

    CA_FUNCTION(get)
    {
        caValue* table = INPUT(0);
        caValue* key = INPUT(1);
        caValue* value = hashtable_t::get_value(table, key);
        if (value == NULL) {
            std::string msg = "Key not found: " + to_string(key);
            return RAISE_ERROR(msg.c_str());
        }
        copy(value, OUTPUT);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, insert, "Map.add(self :out, any, any)");
        import_function(kernel, contains, "Map.contains(self, any) -> bool");
        import_function(kernel, remove, "Map.remove(self :out, any)");
        import_function(kernel, get, "Map.get(self, any) -> any");
    }
}
} // namespace circa
