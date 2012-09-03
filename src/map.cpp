// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "map.h"
#include "types/hashtable.h"

namespace circa {
}

extern "C" {

using namespace circa;

void circa_set_map(caValue* map)
{
    make(TYPES.map, map);
}

caValue* circa_map_insert(caValue* map, caValue* key)
{
    return circa::hashtable_t::table_insert(map, key, false);
}
caValue* circa_map_get(caValue* map, caValue* key)
{
    return circa::hashtable_t::get_value(map, key);
}

}
