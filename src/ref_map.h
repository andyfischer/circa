#ifndef CIRCA_REFERENCE_MAP_INCLUDED
#define CIRCA_REFERENCE_MAP_INCLUDED

#include "common_headers.h"

#include "reference.h"

namespace circa {

struct ReferenceMap
{
    std::map<Reference,Reference> _map;

    typedef std::map<Reference,Reference>::iterator iterator;

    explicit ReferenceMap() { }
    Reference& operator[](Term* key) {
        return _map[key];
    }
    Term* operator[](Term* key) const {
        if (!contains(key))
            return NULL;
        return _map.find(key)->second.get();
    }

    bool contains(Term* key) const {
        return _map.find(key) != _map.end();
    }

    // If 'key' exists in this map, return the associated term.
    // Otherwise, return 'key' back to them.
    Term* getRemapped(Term* key) const {
        if (contains(key))
            return operator[](key);
        else
            return key;
    }

    iterator begin() {
        return _map.begin();
    }

    iterator end() {
        return _map.end();
    }
};

} // namespace circa

#endif
