// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <vector>

#include "weak_ptrs.h"

namespace circa {

std::vector<void*> g_everyWeakPtr;

WeakPtr weak_ptr_create(void* address)
{
    // Make sure we don't give out a value of 0, because this means null.
    if (g_everyWeakPtr.size() == 0)
        g_everyWeakPtr.push_back(NULL);

    g_everyWeakPtr.push_back(address);
    return (int) g_everyWeakPtr.size() - 1;
}

void* get_weak_ptr(WeakPtr ptr)
{
    if (ptr >= g_everyWeakPtr.size())
        return NULL;
    return g_everyWeakPtr[ptr];
}

void weak_ptr_set_null(WeakPtr ptr)
{
    if (ptr == 0)
        return;
    g_everyWeakPtr[ptr] = NULL;
}
bool is_weak_ptr_null(WeakPtr ptr)
{
    if (ptr == 0)
        return true;
    return ptr == 0;
}

}
