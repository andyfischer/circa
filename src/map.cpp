// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "map.h"
#include "types/hashtable.h"

namespace circa {
}

extern "C" {

using namespace circa;

void circa_set_map(caValue* map)
{
    create(TYPES.map, map);
}

void circa_map_insert(caValue* map, caValue* key, caValue* value)
{
    return circa::hashtable_t::table_insert(map, key, false);
}
void circa_map_get(caValue* map, caValue* key, caValue* valueOut)
{
    return circa::hashtable_t::table_get(map, key);
}

}
