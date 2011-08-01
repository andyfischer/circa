// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "debug.h"
#include "term_map.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<std::string, TermPtr> StringToTermMap;
    typedef StringToTermMap::iterator iterator;
    typedef StringToTermMap::const_iterator const_iterator;

    StringToTermMap _map;

    bool contains(std::string const& s) const
    {
        return _map.find(s) != _map.end();

    }
    void bind(Term* term, std::string name);

    void clear()
    {
        _map.clear();
    }

    Term* operator[](std::string const& name) const
    {
        if (DEBUG_TRAP_NAME_LOOKUP)
            ca_assert(false);

        StringToTermMap::const_iterator it = _map.find(name);
        if (it == _map.end())
            return NULL;
        else
            return it->second;
    }

    std::string findName(Term* term) const
    {
        StringToTermMap::const_iterator it;
        for (it = _map.begin(); it != _map.end(); ++it) {
            if (it->second == term) {
                return it->first;
            }
        }

        return "";
    }

    void remove(std::string const& name)
    {
        _map.erase(name);
    }

    void append(TermNamespace& rhs)
    {
        StringToTermMap::const_iterator it;
        for (it = rhs._map.begin(); it != rhs._map.end(); ++it) {
            ca_assert(it->second != NULL);
            _map[it->first] = it->second;
        }
    }

    iterator begin() { return _map.begin(); }
    iterator end() { return _map.end(); }
    const_iterator begin() const { return _map.begin(); }
    const_iterator end() const { return _map.end(); }
    const_iterator find(std::string const& name) const { return _map.find(name); }

    void remapPointers(TermMap const& remapping);
};

} // namespace circa
