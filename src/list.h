// Copyright 2008 Andrew Fischer

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "branch.h"
#include "ref_list.h"

namespace circa {

struct List
{
private:
    ReferenceList items;
public:

    // Default constructor
    List() {}

    // Copy constructor
    List(List const& copy);

    int count() const { return items.count(); }
    Term* get(int index) const { return items.get(index); }
    Term* operator[] (int index) const { return this->get(index); }

    Term* append(Term* term);
    Term* appendSlot(Term* type);

    Term* append(std::string const& str);

    void remove(int index);
    void clear();

    ReferenceList toReferenceList() const;
};

std::string List__toString(Term* caller);
bool is_list(Term* term);
List& as_list(Term* term);
List& as_list_unchecked(Term* term);
void initialize_list_functions(Branch* kernel);

} // namespace circa

#endif
