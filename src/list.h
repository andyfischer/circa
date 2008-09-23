// Copyright 2008 Paul Hodge

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

    int count() const { return items.count(); }
    Term* get(int index) const { return items.get(index); }
    Term* operator[] (int index) const{ return this->get(index); }
    Term* append(Term* term);
    Term* appendSlot(Term* type);
    void clear();
    ReferenceList toReferenceList();
};

List& as_list(Term* term);

} // namespace circa

#endif
