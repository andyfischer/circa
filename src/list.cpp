// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_importing.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "term_pointer_iterator.h"
#include "type.h"
#include "values.h"

namespace circa {

List::List(List const& copy)
  : _signature(LIST_TYPE_SIGNATURE)
{
    for (int i=0; i < copy.count(); i++) {
        Term* term = appendSlot(copy[i]->type);
        duplicate_value(copy[i], term);
    }
}

List& List::operator=(List const& b)
{
    // If all our types match, then don't replace terms.
    bool typesMatch = true;

    if (count() != b.count())
        typesMatch = false;

    if (typesMatch) {
        for (int i=0; i < b.count(); i++) {
            if (get(i)->type != b[i]->type) {
                typesMatch = false;
                break;
            }
        }
    }

    if (typesMatch) {
        // Copy w/o creating new terms
        for (int i=0; i < b.count(); i++) {
            assign_value(b[i], get(i));
        }
    } else {
        clear();
        for (int i=0; i < b.count(); i++) {
            Term* term = appendSlot(b[i]->type);
            assign_value(b[i], term);
        }
    }

    return *this;
}

Term*
List::append(Term* term)
{
    assert(term != NULL);
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

void* List::alloc(Term* typeTerm)
{
    List *value = new List();

    // create a slot for each field
    Type& type = as_type(typeTerm);
    int numFields = (int) type.fields.size();

    for (int f=0; f < numFields; f++)
        value->appendSlot(type.fields[f].type);

    return value;
}

void List::dealloc(void* data)
{
    delete (List*) data;
}

std::string List::to_string(Term* caller)
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
    return ((List*) term->value)->_signature == LIST_TYPE_SIGNATURE;
}

List& as_list(Term* term)
{
    assert(is_list(term));
    assert(term->value != NULL);
    return *((List*) term->value);
}

List& as_list_unchecked(Term* term)
{
    assert(term->value != NULL);
    return *((List*) term->value);
}

class ListPointerIterator : public PointerIterator
{
private:
    List* _list;
    int _index;
    PointerIterator* _nestedIterator;
public:
    ListPointerIterator(List* value)
      : _list(value), _index(0), _nestedIterator(NULL)
    {
        if (_list->items.count() == 0) {
            // Already finished
            _list = NULL;
        } else {
            _nestedIterator = new TermExternalPointersIterator(_list->items[0]);
            postAdvance();
            while (!finished() && !shouldExposePointer(current()))
                internalAdvance();
        }
    }

    virtual Term* current()
    {
        assert(!finished());
        return _nestedIterator->current();
    }
    virtual void advance()
    {
        assert(!finished());
        internalAdvance();
        while (!finished() && !shouldExposePointer(current()))
            internalAdvance();
    }
    virtual bool finished()
    {
        return _list == NULL;
    }

private:
    bool shouldExposePointer(Term* term)
    {
        return term != NULL;
    }
    void internalAdvance()
    {
        assert(!finished());
        _nestedIterator->advance();
        postAdvance();
    }

    void postAdvance()
    {
        while (_nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;

            _index++;
            if (_index >= (int) _list->items.count()) {
                _list = NULL;
                return;
            }

            _nestedIterator = new TermExternalPointersIterator(_list->items[_index]);
        }
    }
};

PointerIterator* List::start_pointer_iterator(Term* term)
{
    return new ListPointerIterator(&as_list(term));
}

PointerIterator* start_list_pointer_iterator(List* list)
{
    return new ListPointerIterator(list);
}

} // namespace circa
