// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <vector>

namespace circa {

struct TermList
{
    std::vector<Term*> _terms;

    TermList();
    TermList(Term* t1);
    TermList(Term* t1, Term* t2);
    TermList(Term* t1, Term* t2, Term* t3);
    TermList(Term* t1, Term* t2, Term* t3, Term* t4);
    TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5);
    TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5, Term* t6);

    int length() const { return _terms.size(); }
    Term* operator[] (int index) const;
    bool empty() const { return length() == 0; }
    bool contains(Term* t) const;

    void setAt(int index, Term* term);
    void append(Term* term);
    void appendUnique(Term* term);
    void appendAll(TermList const& list);
    void remove(int i);
    void remove(Term* term);
    void resize(int s);
    void clear();
    void insert(int index, Term* term);
};

} // namespace circa
