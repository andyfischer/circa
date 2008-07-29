#ifndef CIRCA__TERM_NAMESPACE__INCLUDED
#define CIRCA__TERM_NAMESPACE__INCLUDED

#include "common_headers.h"

struct Term;

struct TermNamespace
{
    std::map<string, Term*> _map;

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
};

#endif
