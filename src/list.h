// Copyright 2008 Paul Hodge

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "branch.h"
#include "ref_list.h"

namespace circa {

// This is a dumb thing that allows for some runtime type assertion.
const int LIST_TYPE_SIGNATURE = 0x91917575;

struct List
{
    int signature;
    ReferenceList items;

    // Default constructor
    List() : signature(LIST_TYPE_SIGNATURE) {}

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

    static void* alloc(Term* typeTerm);
    static void dealloc(void* data);
    static PointerIterator* start_pointer_iterator(Term* term);
};

std::string List__toString(Term* caller);
bool is_list(Term* term);
List& as_list(Term* term);
List& as_list_unchecked(Term* term);
void initialize_list_functions(Branch* kernel);

PointerIterator* start_list_pointer_iterator(List* list);

} // namespace circa

#endif
