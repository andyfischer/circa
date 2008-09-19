#ifndef CIRCA__TERM_LIST__INCLUDED
#define CIRCA__TERM_LIST__INCLUDED

#include "common_headers.h"

namespace circa {

struct TermMap;

struct ReferenceList
{
private:
    std::vector<Term*> _items;
 
public:
    explicit ReferenceList() { }

    // Convenience constructors
    explicit ReferenceList(Term* term) {
        _items.push_back(term);
    }
    ReferenceList(Term* term1, Term* term2) {
        _items.push_back(term1);
        _items.push_back(term2);
    }
    ReferenceList(Term* term1, Term* term2, Term* term3) {
        _items.push_back(term1);
        _items.push_back(term2);
        _items.push_back(term3);
    }

    int count() const { return (int) _items.size(); }
    void append(Term* term) { _items.push_back(term); }
    void appendAll(ReferenceList const& list);
    void setAt(int index, Term* term)
    {
        // Make sure there are enough blank elements in the list
        while (_items.size() <= index) {
            _items.push_back(NULL);
        }

        _items[index] = term;
    }
    void clear() { _items.clear(); }
    Term* get(int index) const
    {
        if (index >= _items.size())
            return NULL;
        return _items[index];
    }
    Term* operator[](int index) const { return get(index); }
    void remapPointers(TermMap const& map);
};

} // namespace circa

#endif
