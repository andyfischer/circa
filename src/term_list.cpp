// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "heap_debugging.h"

#include "term_list.h"

namespace circa {

TermList::TermList()
{
}
TermList::TermList(Term* t1)
{
    resize(1);
    setAt(0, t1);
}
TermList::TermList(Term* t1, Term* t2)
{
    resize(2);
    setAt(0, t1);
    setAt(1, t2);
}
TermList::TermList(Term* t1, Term* t2, Term* t3)
{
    resize(3);
    setAt(0, t1);
    setAt(1, t2);
    setAt(2, t3);
}
TermList::TermList(Term* t1, Term* t2, Term* t3, Term* t4)
{
    resize(4);
    setAt(0, t1);
    setAt(1, t2);
    setAt(2, t3);
    setAt(3, t4);
}
TermList::TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5)
{
    resize(5);
    setAt(0, t1);
    setAt(1, t2);
    setAt(2, t3);
    setAt(3, t4);
    setAt(4, t5);
}
TermList::TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5, Term* t6)
{
    resize(6);
    setAt(0, t1);
    setAt(1, t2);
    setAt(2, t3);
    setAt(3, t4);
    setAt(4, t5);
    setAt(5, t6);
}

void
TermList::set(Term* t1)
{
    resize(1);
    setAt(0, t1);
}
void
TermList::set(Term* t1, Term* t2)
{
    resize(2);
    setAt(0, t1);
    setAt(1, t2);
}
void
TermList::set(Term* t1, Term* t2, Term* t3)
{
    resize(3);
    setAt(0, t1);
    setAt(1, t2);
    setAt(2, t3);
}

void
TermList::setAt(int index, Term* term)
{
    _terms[index] = term;
}

void
TermList::append(Term* term)
{
    _terms.push_back(term);
}

void
TermList::appendUnique(Term* term)
{
    for (int i=0; i < length(); i++) {
        if (_terms[i] == term)
            return;
    }
    debug_assert_valid_object(term, TERM_OBJECT);
    _terms.push_back(term);
}

void
TermList::appendAll(TermList const& list)
{
    ca_assert(&list != this);
    for (int i=0; i < list.length(); i++)
        append(list[i]);
}

void
TermList::prepend(Term* term)
{
    insert(0, term);
}

void
TermList::insert(int index, Term* term)
{
    _terms.resize(_terms.size() + 1);
    for (int i = (int) _terms.size() - 1; i > index; i--)
        _terms[i] = _terms[i - 1];
    _terms[index] = term;
}

void
TermList::remove(int i)
{
    ca_assert(int(_terms.size()) > i);

    int replace = int(_terms.size() - 1);
    if (replace > i) {
        _terms[i] = _terms[replace];
    }
    _terms.resize(replace);
}

void
TermList::remove(Term* term)
{
    int numRemoved = 0;
    for (size_t i=0; i < _terms.size(); i++) {

        if (_terms[i] == term) {
            numRemoved++;
        } else if (numRemoved > 0) {
            _terms[i - numRemoved] = _terms[i];
        }
    }

    _terms.resize(_terms.size() - numRemoved);
}

void TermList::pop()
{
    ca_assert(length() > 0);
    resize(length() - 1);
}

void
TermList::resize(int s)
{
    _terms.resize(s);
}

void
TermList::clear()
{
    _terms.clear();
}

Term*
TermList::operator[] (int index) const
{
    ca_assert(index < int(_terms.size()));
    return _terms[index];
}

bool
TermList::contains(Term* t) const
{
    for (size_t i=0; i < _terms.size(); i++)
        if (_terms[i] == t)
            return true;
    return false;
}

} // namespace circa
