// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#if CIRCA_ENABLE_HEAP_DEBUGGING
#include <map>
#endif

#include "build_options.h"
#include "kernel.h"
#include "heap_debugging.h"
#include "term.h"

namespace circa {

#if CIRCA_ENABLE_HEAP_DEBUGGING

bool types_may_overlap(int typeSet)
{
    if (typeSet == (TERM_OBJECT + TAGGED_VALUE_OBJECT))
        return true;
    return false;
}

// Wrap the global map in a function to avoid static initialization.
std::map<void*, HeapEntry>& heap_map()
{
    static std::map<void*, HeapEntry> *g_heap = new std::map<void*, HeapEntry>();
    return *g_heap;
}

HeapEntry* find_heap_entry(void* addr)
{
    std::map<void*, HeapEntry>::iterator it = heap_map().find(addr);
    if (it == heap_map().end()) return NULL;
    return &it->second;
}

void debug_register_valid_object(void* addr, int type)
{
    if (SHUTTING_DOWN)
        return;

    HeapEntry* entry = find_heap_entry(addr);

    if (entry != NULL) {
        bool invalidOverlap = (entry->type & type)
            || !types_may_overlap(entry->type | type);

        if (invalidOverlap) {
            std::stringstream err;
            err << "Double register at address: 0x" << size_t(addr);
            err << ", with type " << type;
            internal_error(err.str().c_str());
        }
    } else {
        entry = &heap_map()[addr];
    }

    entry->type |= type;
}

void debug_unregister_valid_object(void* addr, int type)
{
    if (SHUTTING_DOWN)
        return;

    debug_assert_valid_object(addr, type);

    HeapEntry* entry = find_heap_entry(addr);
    if (entry == NULL) {
        std::stringstream err;
        err << "unregister_valid_object failed (type = " << type
            << "), nothing registered at addr 0x"
            << std::hex << size_t(addr);
        internal_error(err.str().c_str());
    }
    if ((entry->type & type) == 0) {
        std::stringstream err;
        err << "unregister_valid_object failed, type not registered here (expected "
            << type << ", found " << entry->type << ")";
        internal_error(err.str().c_str());
    }
    entry->type = entry->type & ~type;

    if (entry->type == 0)
        heap_map().erase(addr);
}

void debug_assert_valid_object(void* addr, int type)
{
    if (!STATIC_INITIALIZATION_FINISHED || SHUTTING_DOWN)
        return;

    if (addr == NULL) return;

    HeapEntry* entry = find_heap_entry(addr);
    if (entry == NULL) {
        std::stringstream err;
        err << "assert_valid_object failed (type = "
            << type << "), nothing registered at addr 0x"
            << std::hex << size_t(addr);
        internal_error(err.str().c_str());
    }
    if ((entry->type & type) == 0) {
        std::stringstream err;
        err << "assert_valid_object failed, type not registered here (expected "
            << type << ", found " << entry->type << ")";
        internal_error(err.str().c_str());
    }
}

bool debug_is_object_valid(void* obj, int type)
{
    if (obj == NULL) return false;
    HeapEntry* entry = find_heap_entry(obj);
    if (entry == NULL) return false;
    if (type != entry->type) return false;
    return true;
}

HeapTracker::HeapTracker(int type)
{
    _type = type;
    debug_register_valid_object(this, _type);
}
HeapTracker::~HeapTracker()
{
    debug_unregister_valid_object(this, _type);
}

#else

bool debug_is_object_valid(void* obj, int type)
{
    return true;
}

#endif

void assert_valid_term(Term* term)
{
    debug_assert_valid_object(term, TERM_OBJECT);
}

} // namespace circa
