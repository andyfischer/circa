// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/hashtable.h"

namespace circa {
namespace hashtable_members_function {

    CA_FUNCTION(contains)
    {
        Value* value = hashtable_t::get_value(INPUT(0), INPUT(1));
        set_bool(OUTPUT, value != NULL);
    }

    CA_FUNCTION(insert)
    {
        Value key, value;
        consume_input(CALLER, 0, OUTPUT);
        consume_input(CALLER, 1, &key);
        consume_input(CALLER, 2, &value);
        hashtable_t::table_insert(OUTPUT, &key, &value, true, true);
    }

    CA_FUNCTION(remove)
    {
        consume_input(CALLER, 0, OUTPUT);
        hashtable_t::table_remove(OUTPUT, INPUT(1));
    }

    CA_FUNCTION(get)
    {
        Value* table = INPUT(0);
        Value* key = INPUT(1);
        Value* value = hashtable_t::get_value(table, key);
        if (value == NULL)
            return error_occurred(CONTEXT, CALLER, "Key not found: " + to_string(key));
        copy(value, OUTPUT);
    }

    void setup(Branch& kernel)
    {
        Type* hashType = unbox_type(kernel["Map"]);

        Term* map_add = import_member_function(hashType, insert,
                "add(Map, any, any) -> Map");
        function_set_use_input_as_output(map_add, 0, true);
        import_member_function(hashType, contains, "contains(Map, any) -> bool");
        Term* map_remove = import_member_function(hashType, remove,
                "remove(Map, any) -> Map");
        function_set_use_input_as_output(map_remove, 0, true);
        import_member_function(hashType, get, "get(Map, any) -> any");
    }
}
} // namespace circa
