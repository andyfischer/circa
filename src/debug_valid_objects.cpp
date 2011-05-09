// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#if CIRCA_VALID_OBJECT_CHECKING
#include <map>
#endif

#include "build_options.h"
#include "builtins.h"
#include "errors.h"
#include "debug_valid_objects.h"
#include "term.h"

namespace circa {

#if CIRCA_VALID_OBJECT_CHECKING

std::map<void*, int>& address_to_type()
{
    static std::map<void*, int> *g_addressToType = new std::map<void*,int>;
    return *g_addressToType;
}

void debug_register_valid_object(void* obj, int type)
{
    if (SHUTTING_DOWN)
        return;

    bool alreadyValid = address_to_type().find(obj) != address_to_type().end();
    if (alreadyValid) {
        std::stringstream err;
        err << "Double register at address: " << size_t(obj);
        internal_error(err.str().c_str());
    }
    address_to_type()[obj] = type;
}

void debug_register_valid_object_ignore_dupe(void* obj, int type)
{
    address_to_type()[obj] = type;
}

void debug_unregister_valid_object(void* obj)
{
    if (SHUTTING_DOWN)
        return;

    bool valid = address_to_type().find(obj) != address_to_type().end();
    if (!valid) {
        std::stringstream err;
        err << "Freed unregistered address: " << size_t(obj);
        internal_error(err.str().c_str());
    }
    address_to_type().erase(obj);
}

void debug_assert_valid_object(void* obj, int type)
{
    if (SHUTTING_DOWN)
        return;

    if (obj == NULL) return;
    bool valid = address_to_type().find(obj) != address_to_type().end();
    if (!valid) {
        std::stringstream err;
        err << "assert_valid_object failed, nothing registered at addr 0x"
            << std::hex << size_t(obj);
        internal_error(err.str().c_str());
    }
    int existingType = address_to_type()[obj];
    if (type != existingType) {
        std::stringstream err;
        err << "assert_valid_object failed, type mismatch (expected " << type
            << ", found " << existingType << ")";
        internal_error(err.str().c_str());
    }
}

#endif

bool debug_is_object_valid(void* obj, int type)
{
    #if CIRCA_VALID_OBJECT_CHECKING
        if (obj == NULL) return false;
        bool valid = address_to_type().find(obj) != address_to_type().end();
        if (!valid) return false;
        int existingType = address_to_type()[obj];
        if (type != existingType) return false;
        return true;
    #else
        return true;
    #endif
}

void assert_valid_term(Term* term)
{
    debug_assert_valid_object(term, TERM_OBJECT);
}

bool debug_is_term_pointer_valid(Term* term)
{
    return debug_is_object_valid(term, TERM_OBJECT);
}

} // namespace circa
