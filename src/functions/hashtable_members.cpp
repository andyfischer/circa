// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/hashtable.h"

namespace circa {
namespace hashtable_members_function {

    CA_FUNCTION(contains)
    {
        TaggedValue* value = hashtable_t::get_value(INPUT(0), INPUT(1));
        set_bool(OUTPUT, value != NULL);
    }

    CA_FUNCTION(insert)
    {
        TaggedValue key, value;
        consume_input(CONTEXT, CALLER, 0, OUTPUT);
        consume_input(CONTEXT, CALLER, 1, &key);
        consume_input(CONTEXT, CALLER, 2, &value);
        hashtable_t::table_insert(OUTPUT, &key, &value, true, true);
    }

    CA_FUNCTION(remove)
    {
        consume_input(CONTEXT, CALLER, 0, OUTPUT);
        hashtable_t::table_remove(OUTPUT, INPUT(1));
    }

    CA_FUNCTION(get)
    {
        TaggedValue* table = INPUT(0);
        TaggedValue* key = INPUT(1);
        TaggedValue* value = hashtable_t::get_value(table, key);
        if (value == NULL)
            return error_occurred(CONTEXT, CALLER, "Key not found: " + to_string(key));
        copy(value, OUTPUT);
    }

    void setup(Branch& kernel)
    {
        Type* hashType = unbox_type(kernel["Map"]);

        import_member_function(hashType, insert,
                "add(Map :implied_rebind, any, any) -> Map");
        import_member_function(hashType, contains, "contains(Map, any) -> bool");
        import_member_function(hashType, remove,
                "remove(Map :implied_rebind, any) -> Map");
        import_member_function(hashType, get, "get(Map, any) -> any");
    }
}
} // namespace circa
