// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/hashtable.h"

namespace circa {
namespace hashtable_methods_function {

    CA_FUNCTION(contains)
    {
        TaggedValue* value = hashtable_t::get_value(INPUT(0), INPUT(1));
        set_bool(OUTPUT, value != NULL);
    }

    CA_FUNCTION(insert)
    {
        TaggedValue key, value;
        CONSUME_INPUT(0, OUTPUT);
        CONSUME_INPUT(1, &key);
        CONSUME_INPUT(2, &value);
        hashtable_t::table_insert(OUTPUT, &key, &value, true, true);
    }

    CA_FUNCTION(remove)
    {
        CONSUME_INPUT(0, OUTPUT);
        hashtable_t::table_remove(OUTPUT, INPUT(1));
    }

    CA_FUNCTION(get)
    {
        TaggedValue* table = INPUT(0);
        TaggedValue* key = INPUT(1);
        TaggedValue* value = hashtable_t::get_value(table, key);
        if (value == NULL) {
            std::string msg = "Key not found: " + to_string(key);
            return RAISE_ERROR(msg.c_str());
        }
        copy(value, OUTPUT);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, insert,
                "Map.add(self :implied_rebind, any, any) -> Map");
        import_function(kernel, contains, "Map.contains(self, any) -> bool");
        import_function(kernel, remove,
                "Map.remove(self :implied_rebind, any) -> Map");
        import_function(kernel, get, "Map.get(self, any) -> any");
    }
}
} // namespace circa
