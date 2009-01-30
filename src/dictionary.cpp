// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "dictionary.h"
#include "runtime.h"
#include "term.h"
#include "values.h"

namespace circa {

Term* Dictionary::get(std::string const& name) const
{
    RefDictionary::const_iterator it = _dict.find(name);

    if (it == _dict.end())
        return NULL;

    return it->second;
}

Term* Dictionary::addSlot(std::string const& name, Term* type)
{
    assert(get(name) == NULL);

    Term* newTerm = create_value(NULL, type);
    _dict[name] = newTerm;
    return newTerm;
}

void Dictionary::remove(std::string const& name)
{
    Term* term = get(name);
    assert(term != NULL);
    delete_term(term);
    _dict.erase(_dict.find(name));
}

void Dictionary::clear()
{
    RefDictionary::iterator it;
    for (it = _dict.begin(); it != _dict.end(); ++it) {
        delete it->second;
    }
    _dict.clear();
}

} // namespace circa
