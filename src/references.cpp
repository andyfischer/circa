// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

void remove_referencer(Term* term, Ref* ref)
{
    std::vector<Ref*>::iterator it;
    for (it = term->refs.begin(); it != term->refs.end();) {
        if (*it == ref) {
            it = term->refs.erase(it);
        } else {
            ++it;
        }
    }

    if (term->refs.size() == 0)
        delete_term(term);
}

} // namespace circa
