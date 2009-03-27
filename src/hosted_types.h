// Copyright 2009 Andrew Fischer

#ifndef CIRCA_HOSTED_TYPES_INCLUDED
#define CIRCA_HOSTED_TYPES_INCLUDED

#include "circa.h"

namespace circa {

struct StringList {

    Branch data;

    void append(std::string const& str) {
        string_value(&data, str);
    }

    std::string& operator[](int index) {
        return as_string(data[index]);
    }

    int length() {
        return data.numTerms();
    }
};

}

#endif
