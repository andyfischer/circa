#ifndef CIRCA__TERM_NAMESPACE__INCLUDED
#define CIRCA__TERM_NAMESPACE__INCLUDED

#include "common_headers.h"

#include "term_map.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<string, Term*> StringToTermMap;
    StringToTermMap _map;

    bool contains(std::string s)
    {
        return _map.find(s) != _map.end();

    }
    void bind(Term* term, string name)
    {
        _map[name] = term;
    }
    void clear()
    {
        _map.clear();
    }

    Term* operator[](string name)
    {
        return _map[name];
    }

    std::string findName(Term* term)
    {
        StringToTermMap::iterator it;
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
        for (it = _map.begin(); it != _map.end(); ++it) {
            Term* replacement = remapping[it->second];
            if (replacement != it->second) {
                if (replacement == NULL) {
                    _map.erase(it); // don't touch 'it' after this
                }
                else {
                    _map[it->first] = replacement;
                }
            }
        }
    }
};

} // namespace circa

#endif
