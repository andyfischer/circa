// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "type.h"
#include "values.h"

namespace circa {

List::List(List const& copy)
{
    for (int i=0; i < copy.count(); i++) {
        Term* term = appendSlot(copy[i]->type);
        duplicate_value(copy[i], term);
    }
}

Term*
List::append(Term* term)
{
    Term* newTerm = create_value(NULL, term->type);
    recycle_value(term, newTerm);
    this->items.append(newTerm);
    return newTerm;
}

Term*
List::appendSlot(Term* type)
{
    Term* newTerm = create_value(NULL, type);
    this->items.append(newTerm);
    return newTerm;
}

Term*
List::append(std::string const& str)
{
    Term *term = create_value(NULL, STRING_TYPE);
    as_string(term) = str;
    items.append(term);
    return term;
}

void
List::remove(int index)
{
    delete items[index];
    items.remove(index);
}

void
List::clear()
{
    for (unsigned int i=0; i < items.count(); i++)
        delete items[i];
    items.clear();
}

std::string List__toString(Term* caller)
{
    std::stringstream out;
    List& list = as_list(caller);
    out << "[";
    bool first_item = true;
    for (int i=0; i < list.count(); i++) {
        if (!first_item) out << ", ";
        out << list[i]->toString();
        first_item = false;
    }
    out << "]";
    return out.str();
}

bool is_list(Term* term)
{
    return term->type == LIST_TYPE;
}

List& as_list(Term* term)
{
    assert_type(term, LIST_TYPE);
    assert(term->value != NULL);
    return *((List*) term->value);
}

List& as_list_unchecked(Term* term)
{
    assert(term->value != NULL);
    return *((List*) term->value);
}

} // namespace circa
