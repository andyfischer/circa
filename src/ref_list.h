// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TERM_LIST_INCLUDED
#define CIRCA_TERM_LIST_INCLUDED

// RefList
//
// A list of weak Term pointers.
//

#include "common_headers.h"

#include "ref.h"

namespace circa {

struct RefList
{
private:
    std::vector<Ref> _items;
 
public:
    explicit RefList() { }

    // Convenience constructors
    explicit RefList(Term* term) {
        append(term);
    }
    RefList(Term* term1, Term* term2) {
        append(term1);
        append(term2);
    }
    RefList(Term* term1, Term* term2, Term* term3) {
        append(term1);
        append(term2);
        append(term3);
    }
    RefList(Term* term1, Term* term2, Term* term3, Term* term4) {
        append(term1);
        append(term2);
        append(term3);
        append(term4);
    }
    RefList(Term* term1, Term* term2, Term* term3, Term* term4, Term* term5) {
        append(term1);
        append(term2);
        append(term3);
        append(term4);
        append(term5);
    }
    RefList(Term* term1, Term* term2, Term* term3, Term* term4, Term* term5, Term* term6) {
        append(term1);
        append(term2);
        append(term3);
        append(term4);
        append(term5);
        append(term6);
    }
    RefList(Term* term1, Term* term2, Term* term3, Term* term4, Term* term5, Term* term6, Term* term7) {
        append(term1);
        append(term2);
        append(term3);
        append(term4);
        append(term5);
        append(term6);
        append(term7);
    }
    RefList(Term* term1, Term* term2, Term* term3, Term* term4, Term* term5, Term* term6, Term* term7, Term* term8) {
        append(term1);
        append(term2);
        append(term3);
        append(term4);
        append(term5);
        append(term6);
        append(term7);
        append(term8);
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

    void appendAll(RefList const& list);
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
        std::vector<Ref>::iterator it;

        for (it = _items.begin(); it != _items.end(); ) {

            if (*it == term)
                it = _items.erase(it);
            else
                ++it;
        }
    }

    void remove(int index)
    {
        _items.erase(_items.begin() + index);
    }

    void removeNulls()
    {
        remove((Term*)NULL);
    }

    void clear() { _items.clear(); }

    Term* get(unsigned int index) const
    {
        if (index >= _items.size())
            return NULL;
        return _items[index];
    }

    bool contains(Term* term) const
    {
        for (unsigned int i=0; i < count(); i++)
            if (get(i) == term)
                return true;
        return false;
    }

    Term* operator[](unsigned int index) const { return get(index); }

    Ref& operator[](unsigned int index)
    {
        return _items[index];
    }

    bool operator==(RefList const& b)
    {
        if (count() != b.count())
            return false;

        for (unsigned int i=0; i < count(); i++)
            if (get(i) != b.get(i))
                return false;

        return true;
    }

    int findIndex(Term* term)
    {
        for (unsigned int i=0; i < count(); i++)
            if (_items[i] == term)
                return i;
        return -1;
    }

    void remapPointers(ReferenceMap const& map);
};

} // namespace circa

#endif
