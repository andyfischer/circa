// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <vector>

#include "weak_ptrs.h"

namespace circa {

std::vector<void*> g_everyWeakPtr;

WeakPtr weak_ptr_create(void* address)
{
    g_everyWeakPtr.push_back(address);
    return (int) g_everyWeakPtr.size();
}

void* get_weak_ptr(WeakPtr ptr)
{
    if (ptr >= g_everyWeakPtr.size())
        return NULL;
    return g_everyWeakPtr[ptr];
}

void weak_ptr_set_null(WeakPtr ptr)
{
    g_everyWeakPtr[ptr] = NULL;
}

}
