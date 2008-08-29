#ifndef CIRCA__TERM_SET__INCLUDED
#define CIRCA__TERM_SET__INCLUDED

#include "common_headers.h"

namespace circa {

class Term;

struct TermSet
{
    std::set<Term*> _items;

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
};

} // namespace circa

#endif
