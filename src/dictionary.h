// Copyright 2008 Andrew Fischer

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
    Branch _branch;
public:

    // Default constructor
    Dictionary() {}

    Term* operator[] (std::string const& name) const { return this->get(name); }
    Term* get(std::string const& name) const {
        RefDictionary::const_iterator it = _dict.find(name);

        if (it == _dict.end())
            return NULL;

        return it->second;
    }

    Term* addSlot(std::string const& name, Term* type);

    void clear()
    {
        _dict.clear();
        _branch.clear();
    }
};

} // namespace circa

#endif
