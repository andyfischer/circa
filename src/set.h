// Copyright 2008 Andrew Fischer

#ifndef CIRCA_SET_INCLUDED
#define CIRCA_SET_INCLUDED

namespace circa {

struct Set {
    // TODO: more efficient data structure
    std::vector<Term*> members;

    bool contains(Term* value);
    void add(Term* value);
    void remove(Term* value);
    void clear();
    static void hosted_add(Term* caller);
    static void hosted_remove(Term* caller);
    static std::string to_string(Term* caller);
};

} // namespace circa

#endif
