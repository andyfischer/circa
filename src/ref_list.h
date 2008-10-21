#ifndef CIRCA_TERM_LIST_INCLUDED
#define CIRCA_TERM_LIST_INCLUDED

#include "common_headers.h"

#include "pointer_visitor.h"

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

        // Check for a previous bug
        if (_items.size() > 1000000)
            throw std::runtime_error("too many items");
    }

    // Add 'term' to this list, if it's not in the list already
    void appendUnique(Term* term)
    {
        for (unsigned int i=0; i < count(); i++)
            if (get(i) == term)
                return;

        append(term);
    }

    void appendAll(ReferenceList const& list);
    void setAt(unsigned int index, Term* term)
    {
        // Check for a previous bug
        if (index > 1000000)
            throw std::runtime_error("index too big in setAt");

        // Make sure there are enough blank elements in the list
        while (_items.size() <= index) {
            _items.push_back(NULL);
        }

        _items[index] = term;
    }

    // Remove 'term' from this list
    void remove(Term* term)
    {
        std::vector<Term*>::iterator it;

        for (it = _items.begin(); it != _items.end(); ) {

            if (*it == term)
                it = _items.erase(it);
            else
                ++it;
        }
    }

    void removeNulls()
    {
        remove(NULL);
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
    void visitPointers(PointerVisitor &visitor);
};

} // namespace circa

#endif
