#ifndef CIRCA_REF_SET_INCLUDED
#define CIRCA_REF_SET_INCLUDED

#include "common_headers.h"

namespace circa {

struct ReferenceSet
{
    typedef std::set<Term*> TermPtrSet;
    TermPtrSet _items;

    typedef TermPtrSet::iterator iterator;

    iterator begin() {
        return _items.begin();
    }
    iterator end() {
        return _items.end();
    }

    bool contains(Term* term) const {
        return _items.find(term) != _items.end();
    }
    void add(Term* term) {
        _items.insert(term);
    }
    void remove(Term* term) {
        _items.erase(term);
    }
    int count() const {
        return (int) _items.size();
    }
    void clear() {
        _items.clear();
    }
};

} // namespace circa

#endif
