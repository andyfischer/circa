// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_REF_LIST_INCLUDED
#define CIRCA_REF_LIST_INCLUDED

// RefList
//
// A list of Term references

#include "common_headers.h"
#include "references.h"

namespace circa {

struct RefList
{
    std::vector<Ref> _items;
 
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

    void append(Term* term);
    void prepend(Term* term);
    void insert(int index, Term* term);

    // Add 'term' to this list, if it's not in the list already
    void appendUnique(Term* term);

    void appendAll(RefList const& list);
    void setAt(unsigned int index, Term* term);

    // Remove 'term' from this list. 'term' may be NULL.
    void remove(Term* term);
    void remove(int index);

    void removeNulls() { remove((Term*)NULL); }
    void clear() { _items.clear(); }

    int length() const;
    bool empty() const { return _items.empty(); }
    Term* get(unsigned int index) const;

    bool contains(Term* term) const
    {
        for (int i=0; i < length(); i++)
            if (get(i) == term)
                return true;
        return false;
    }

    Term* operator[](unsigned int index) const;
    Ref& operator[](unsigned int index);

    bool operator==(RefList const& b)
    {
        if (length() != b.length())
            return false;

        for (int i=0; i < length(); i++)
            if (get(i) != b.get(i))
                return false;

        return true;
    }

    int findIndex(Term* term)
    {
        for (int i=0; i < length(); i++)
            if (_items[i] == term)
                return i;
        return -1;
    }

    void resize(int newLength);

    void remapPointers(ReferenceMap const& map);
};

void sort_by_name(RefList& list);

} // namespace circa

#endif
