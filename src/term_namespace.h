#ifndef CIRCA__TERM_NAMESPACE__INCLUDED
#define CIRCA__TERM_NAMESPACE__INCLUDED

#include "common_headers.h"

#include "term_map.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<string, Term*> StringToTermMap;
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

    Term* operator[](std::string name) const
    {
        return _map.find(name)->second;
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

    StringToTermMap::iterator begin()
    {
        return _map.begin();
    }

    StringToTermMap::iterator end()
    {
        return _map.end();
    }

    void remapPointers(TermMap const& remapping)
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
