// Copyright 2008 Paul Hodge

#ifndef CIRCA_DICTIONARY_INCLUDED
#define CIRCA_DICTIONARY_INCLUDED

#include "ref_list.h"
#include "branch.h"

namespace circa {

struct Dictionary
{
private:
    typedef std::map<std::string, Term*> RefDictionary;
    RefDictionary _dict;

public:

    // Default constructor
    Dictionary() {}

    bool contains(std::string const& name) const
    {
        return _dict.find(name) != _dict.end();
    }

    Term* operator[] (std::string const& name) const { return this->get(name); }

    Term* get(std::string const& name) const;
    Term* addSlot(std::string const& name, Term* type);
    void remove(std::string const& name);
    void clear();
};

} // namespace circa

#endif
