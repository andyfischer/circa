#ifndef CIRCA_TERM_NAMESPACE_INCLUDED
#define CIRCA_TERM_NAMESPACE_INCLUDED

#include "common_headers.h"

#include "ref_map.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<std::string, Term*> StringToTermMap;
    typedef StringToTermMap::iterator iterator;
    StringToTermMap _map;

    bool contains(std::string s) const
    {
        return _map.find(s) != _map.end();

    }
    void bind(Term* term, std::string name)
    {
        _map[name] = term;
    }

    void clear()
    {
        _map.clear();
    }

    Term* operator[](std::string const& name) const
    {
        return _map.find(name)->second;
    }

    Term*& operator[](std::string const& name)
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

    iterator begin()
    {
        return _map.begin();
    }

    iterator end()
    {
        return _map.end();
    }

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
