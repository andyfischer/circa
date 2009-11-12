// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_TERM_NAMESPACE_INCLUDED
#define CIRCA_TERM_NAMESPACE_INCLUDED

#include "common_headers.h"

#include "debug.h"
#include "metrics.h"
#include "ref_map.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<std::string, Ref> StringToTermMap;
    typedef StringToTermMap::iterator iterator;
    typedef StringToTermMap::const_iterator const_iterator;

    StringToTermMap _map;

    bool contains(std::string s) const
    {
        return _map.find(s) != _map.end();

    }
    void bind(Term* term, std::string name)
    {
        METRIC_NAMESPACE_BINDINGS++;

        _map[name] = term;
    }

    void clear()
    {
        _map.clear();
    }

    Term* operator[](std::string const& name) const
    {
        if (DEBUG_TRAP_NAME_LOOKUP)
            assert(false);

        METRIC_NAMESPACE_LOOKUPS++;

        StringToTermMap::const_iterator it = _map.find(name);
        if (it == _map.end())
            return NULL;
        else
            return it->second;
    }

    Ref& operator[](std::string const& name)
    {
        return _map[name];
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
            _map[it->first] = it->second;
        }
    }

    iterator begin() { return _map.begin(); }
    iterator end() { return _map.end(); }
    const_iterator begin() const { return _map.begin(); }
    const_iterator end() const { return _map.end(); }

    void remapPointers(ReferenceMap const& remapping)
    {
        StringToTermMap::iterator it;
        for (it = _map.begin(); it != _map.end(); ) {
            Term* replacement = remapping.getRemapped(it->second);
            if (replacement != it->second) {
                if (replacement == NULL) {
                    _map.erase(it++);
                    continue;
                }
                else {
                    _map[it->first] = replacement;
                }
            }
            ++it;
        }
    }
};

} // namespace circa

#endif
