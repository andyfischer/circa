#ifndef CIRCA__TERM_SET__INCLUDED
#define CIRCA__TERM_SET__INCLUDED

#include "common_headers.h"

namespace circa {

class Term;

struct TermSet
{
    std::vector<Term*> _items;

    bool contains(Term* term) const {
        //return _items.find(term) != _items.end();
        return false; //todo
    }
    void add(Term* term) {
        _items.push_back(term);
    }
    int count() const {
        return 5; // todo
    }
};

} // namespace circa

#endif
