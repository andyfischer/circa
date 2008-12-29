// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "dictionary.h"
#include "runtime.h"
#include "values.h"

namespace circa {

Term* Dictionary::addSlot(std::string const& name, Term* type)
{
    assert(get(name) == NULL);

    Term* newTerm = create_value(&_branch, type);
    _dict[name] = newTerm;
    return newTerm;
}

} // namespace circa
