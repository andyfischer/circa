
#include "common_headers.h"

#include "errors.h"
#include "term_map.h"

Term&
TermMap::operator[](Term key)
{
    if (!contains(key))
        throw errors::KeyError();

    return _map[key];
}

bool contains(Term key) const
{
    return _map.find(key) != _map.end();
}

Term TermMap::getRemapped(Term key) const
{
    if (contains(key))
        return operator[](key);
    else
        return key;
}
