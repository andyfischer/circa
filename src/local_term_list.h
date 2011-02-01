// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <vector>

namespace circa {

struct LocalTermList
{
    std::vector<Term*> _terms;
    int length() const { return _terms.size(); }
    void setAt(int index, Term* term);
    void append(Term* term);
    void appendUnique(Term* term);
    void remove(int i);
    void remove(Term* term);
    void resize(int s);
    void clear();
    void insert(int index, Term* term);
    Term* operator[] (int index) const;
};

} // namespace circa
