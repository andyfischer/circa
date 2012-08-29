// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <vector>

namespace circa {

struct TermList
{
    std::vector<Term*> _terms;

    explicit TermList();
    explicit TermList(Term* t1);
    explicit TermList(Term* t1, Term* t2);
    explicit TermList(Term* t1, Term* t2, Term* t3);
    explicit TermList(Term* t1, Term* t2, Term* t3, Term* t4);
    explicit TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5);
    explicit TermList(Term* t1, Term* t2, Term* t3, Term* t4, Term* t5, Term* t6);

    void set(Term* t1);
    void set(Term* t1, Term* t2);
    void set(Term* t1, Term* t2, Term* t3);

    int length() const { return (int) _terms.size(); }
    Term* operator[] (int index) const { return get(index); }
    Term* get(int index) const;
    bool empty() const { return length() == 0; }
    bool contains(Term* t) const;

    void setAt(int index, Term* term);
    void append(Term* term);
    void appendUnique(Term* term);
    void appendAll(TermList const& list);
    void prepend(Term* term);
    void insert(int index, Term* term);
    void remove(int i);
    void remove(Term* term);
    void pop();
    void resize(int s);
    void clear();
    void reverse();
};

} // namespace circa
