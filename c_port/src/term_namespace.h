#ifndef CIRCA__TERM_NAMESPACE__INCLUDED
#define CIRCA__TERM_NAMESPACE__INCLUDED

#include "common_headers.h"

namespace circa {

struct TermNamespace
{
    typedef std::map<string, Term*> StringToTermMap;
    StringToTermMap _map;

    bool contains(string s)
    {
        return _map.find(s) != _map.end();

    }
    void bind(Term* term, string name)
    {
        _map[name] = term;
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
};

} // namespace circa

#endif
