// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <cassert>

#include "debug_valid_objects.h"

#if ENABLE_VALID_OBJECT_CHECKING

#include <map>

std::map<void*, int> g_addressToType;

void debug_register_valid_object(void* obj, int type)
{
    bool valid = g_addressToType.find(obj) != g_addressToType.end();
    assert(!valid);
    g_addressToType[obj] = type;
}

void debug_unregister_valid_object(void* obj)
{
    bool valid = g_addressToType.find(obj) != g_addressToType.end();
    assert(valid);
    g_addressToType.erase(obj);
}

void debug_assert_valid_object(void* obj, int type)
{
    if (obj == NULL) return;
    bool valid = g_addressToType.find(obj) != g_addressToType.end();
    assert(valid);
    int existingType = g_addressToType[obj];
    assert(type == existingType);
}

#else

void empty_function_so_that_linker_doesnt_complain() {}

#endif
