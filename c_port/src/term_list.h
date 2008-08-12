#ifndef CIRCA__TERM_LIST__INCLUDED
#define CIRCA__TERM_LIST__INCLUDED

#include "common_headers.h"

namespace circa {

struct TermMap;

struct TermList
{
    std::vector<Term*> _items;

    explicit TermList() { }

    // Convenience constructors
    explicit TermList(Term* term) {
        _items.push_back(term);
    }
    TermList(Term* term1, Term* term2) {
        _items.push_back(term1);
        _items.push_back(term2);
    }
    TermList(Term* term1, Term* term2, Term* term3) {
        _items.push_back(term1);
        _items.push_back(term2);
        _items.push_back(term3);
    }

    int count() const;
    void append(Term* term);
    void setAt(int index, Term* term);
    void clear();
    Term* get(int index) const;
    Term* operator[](int index) const;
    void remap(TermMap& map);
};

void initialize_term_list(Branch* kernel);

} // namespace circa

#endif
