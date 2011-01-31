// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "debug_valid_objects.h"

#include "local_term_list.h"

namespace circa {

void
LocalTermList::appendUnique(Term* term)
{
    for (int i=0; i < length(); i++) {
        if (_terms[i] == term)
            return;
    }
    debug_assert_valid_object(term, TERM_OBJECT);
    _terms.push_back(term);
}

void
LocalTermList::remove(int i)
{
    ca_assert(_terms.size() > i);

    int replace = _terms.size() - 1;
    if (replace > i) {
        _terms[i] = _terms[replace];
    }
    _terms.resize(replace);
}

void
LocalTermList::remove(Term* term)
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

Term*
LocalTermList::operator[](int index)
{
    ca_assert(index < int(_terms.size()));
    return _terms[index];
}

} // namespace circa
