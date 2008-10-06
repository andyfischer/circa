#ifndef CIRCA_TERM_LIST_INCLUDED
#define CIRCA_TERM_LIST_INCLUDED

#include "common_headers.h"

namespace circa {

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

    size_t count() const { return _items.size(); }
    void append(Term* term)
    {
        _items.push_back(term);
        if (_items.size() > 1000000)
            std::cout << "too high in append" << std::endl;
    }
    void appendAll(ReferenceList const& list);
    void setAt(unsigned int index, Term* term)
    {
        if (index > 1000000)
            std::cout << "index too big in setAt" << std::endl;

        // Make sure there are enough blank elements in the list
        while (_items.size() <= index) {
            _items.push_back(NULL);
        }

        _items[index] = term;
    }
    void clear() { _items.clear(); }
    Term* get(unsigned int index) const
    {
        if (index >= _items.size())
            return NULL;
        return _items[index];
    }
    Term* operator[](unsigned int index) const { return get(index); }
    void remapPointers(ReferenceMap const& map);
};

} // namespace circa

#endif
