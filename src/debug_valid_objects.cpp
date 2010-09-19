// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#if ENABLE_VALID_OBJECT_CHECKING
#include <map>
#endif

#include "build_options.h"
#include "errors.h"
#include "debug_valid_objects.h"
#include "term.h"

namespace circa {

#if ENABLE_VALID_OBJECT_CHECKING
std::map<void*, int> *g_addressToType = new std::map<void*,int>;

void debug_register_valid_object(void* obj, int type)
{
    bool alreadyValid = g_addressToType->find(obj) != g_addressToType->end();
    if (alreadyValid) {
        std::stringstream err;
        err << "Double register at address: " << size_t(obj);
        internal_error(err.str().c_str());
    }
    (*g_addressToType)[obj] = type;
}

void debug_unregister_valid_object(void* obj)
{
    bool valid = g_addressToType->find(obj) != g_addressToType->end();
    if (!valid) {
        std::stringstream err;
        err << "Freed unregistered address: " << size_t(obj);
        internal_error(err.str().c_str());
    }
    g_addressToType->erase(obj);
}

void debug_assert_valid_object(void* obj, int type)
{
    if (obj == NULL) return;
    bool valid = g_addressToType->find(obj) != g_addressToType->end();
    if (!valid) {
        std::stringstream err;
        err << "assert_valid_object failed, nothing registered at addr " << size_t(obj);
        internal_error(err.str().c_str());
    }
    int existingType = (*g_addressToType)[obj];
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
    #if ENABLE_VALID_OBJECT_CHECKING
        if (obj == NULL) return false;
        bool valid = g_addressToType->find(obj) != g_addressToType->end();
        if (!valid) return false;
        int existingType = (*g_addressToType)[obj];
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
