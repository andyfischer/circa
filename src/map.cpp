// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "map.h"
#include "types/hashtable.h"

namespace circa {

CIRCA_EXPORT void circa_set_map(caValue* map)
{
    make(TYPES.map, map);
}

CIRCA_EXPORT caValue* circa_map_insert(caValue* map, caValue* key)
{
    return hashtable_t::table_insert(map, key, false);
}
CIRCA_EXPORT caValue* circa_map_get(caValue* map, caValue* key)
{
    return hashtable_t::get_value(map, key);
}

} // namespace circa
