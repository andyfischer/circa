// Copyright 2008 Andrew Fischer

#ifndef CIRCA__LIST__INCLUDED
#define CIRCA__LIST__INCLUDED

#include "ref_list.h"

namespace circa {

struct List
{
private:
    ReferenceList items;
    Branch branch;
public:

    // Default constructor
    List() {}

    // Copy constructor
    List(List const& copy);

    int count() const { return items.count(); }
    Term* get(int index) const { return items.get(index); }
    Term* operator[] (int index) const{ return this->get(index); }
    Term* append(Term* term);
    Term* appendSlot(Term* type);

    void clear()
    {
        items.clear();
        branch.clear();
    }
    ReferenceList toReferenceList()
    {
        ReferenceList result;

        for (unsigned int i=0; i < items.count(); i++)
            result.append(as_ref(items[i]));

        return result;
    }
};

std::string List__toString(Term* caller);
bool is_list(Term* term);
List& as_list(Term* term);
List& as_list_unchecked(Term* term);

} // namespace circa

#endif
