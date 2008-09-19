// Copyright 2008 Paul Hodge

#ifndef CIRCA__VALUE_LIST__INCLUDED
#define CIRCA__VALUE_LIST__INCLUDED

namespace circa {

struct List
{
    TermList items;
    Branch branch;

    int count() const { return items.count(); }
    Term* get(int index) const { return items.get(index); }
    Term* operator[] (int index) const{ return this->get(index); }
    void append(Term* term);
    void clear();
    TermList toReferenceList();
};

List& as_list(Term* term);

void initialize_list(Branch* kernel);


} // namespace circa

#endif
