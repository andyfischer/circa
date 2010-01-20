// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

namespace circa {

extern Type* STRING_TYPE_2;

namespace set_t {
    void add(Branch& branch, Term* value);
}

namespace list_t {
    void append(Branch& branch, Term* value);
    std::string to_string(Term* caller);
}

namespace dict_t {
    std::string to_string(Branch& branch);
}

void initialize_builtin_types();
void setup_builtin_types(Branch& kernel);
void parse_builtin_types(Branch& kernel);

}

#endif
