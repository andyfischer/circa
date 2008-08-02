
#include "common_headers.h"

#include "errors.h"
#include "term_map.h"

Term*&
TermMap::operator[](Term* key)
{
    if (!contains(key))
        throw errors::KeyError("");

    return _map[key];
}

bool
TermMap::contains(Term* key) const
{
    return _map.find(key) != _map.end();
}

Term*
TermMap::getRemapped(Term* key)
{
    if (contains(key))
        return _map[key];
    else
        return key;
}
