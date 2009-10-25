// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_HOSTED_TYPES_INCLUDED
#define CIRCA_HOSTED_TYPES_INCLUDED

#include "circa.h"

namespace circa {

// This file is deprecated, functionality should be moved to cpp_convenience.h

struct StringList {

    Branch data;

    void append(std::string const& str) {
        create_string(data, str);
    }

    std::string& operator[](int index) {
        return as_string(data[index]);
    }

    int length() {
        return data.length();
    }
};

}

#endif
