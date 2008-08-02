#ifndef CIRCA__TERM_MAP__INCLUDED
#define CIRCA__TERM_MAP__INCLUDED

#include "common_headers.h"

struct TermMap
{
    std::map<Term*,Term*> _map;

    explicit TermMap() { }
    Term*& operator[](Term* key);

    bool contains(Term* key) const;

    // If 'key' exists in this map, return the associated term.
    // Otherwise, return 'key' back to them.
    Term* getRemapped(Term* key);
};

#endif
