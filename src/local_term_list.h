// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <vector>

namespace circa {

struct LocalTermList
{
    std::vector<Term*> _terms;
    int length() const { return _terms.size(); }
    void appendUnique(Term* term);
    void remove(int i);
    void remove(Term* term);
    Term* operator[](int index);
};

} // namespace circa
