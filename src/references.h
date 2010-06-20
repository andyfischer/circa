// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

struct Ref
{
    // Super short name for this field because we often need to type it into
    // the debugger.
    Term* t;

    Ref() : t(NULL) {}
    Ref(Term *initial) : t(NULL) { set(initial); }
    Ref(Ref const& copy) : t(NULL) { set(copy.t); }
    ~Ref() { set(NULL); }

    void set(Term* target);

    Ref& operator=(Ref const& rhs) { set(rhs.t); return *this; }
    Ref& operator=(Term* target) { set(target); return *this; }
    bool operator==(Term* _t) const { return _t == t; }
    operator Term*() const { return t; }
    Term* operator->() { return t; }

    static void remap_pointers(Term* term, ReferenceMap const& map);
};

void delete_term(Term* term);
void remap_pointers(Term* term, ReferenceMap const& map);
void remap_pointers(Term* term, Term* original, Term* replacement);
void remap_pointers(Branch& branch, Term* original, Term* replacement);

} // namespace circa
