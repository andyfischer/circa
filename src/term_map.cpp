// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "errors.h"
#include "term_map.h"

namespace circa {

Term*&
TermMap::operator[](Term* key)
{
    return _map[key];
}

Term*
TermMap::operator[](Term* key) const
{
    return _map.find(key)->second;
}

bool
TermMap::contains(Term* key) const
{
    return _map.find(key) != _map.end();
}

Term*
TermMap::getRemapped(Term* key) const
{
    if (contains(key))
        return operator[](key);
    else
        return key;
}

TermMap::iterator
TermMap::begin()
{
    return _map.begin();
}

TermMap::iterator
TermMap::end()
{
    return _map.end();
}

} // namespace circa
