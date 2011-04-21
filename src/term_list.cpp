// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "debug_valid_objects.h"

#include "term_list.h"

namespace circa {

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
TermList::remove(int i)
{
    ca_assert(int(_terms.size()) > i);

    int replace = _terms.size() - 1;
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

void
TermList::insert(int index, Term* term)
{
    _terms.resize(_terms.size() + 1);
    for (size_t i=index; i < _terms.size() - 1; i++)
        _terms[i + 1] = _terms[i];
    _terms[index] = term;
}

Term*
TermList::operator[] (int index) const
{
    ca_assert(index < int(_terms.size()));
    return _terms[index];
}

} // namespace circa
