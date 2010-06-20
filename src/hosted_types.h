// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "circa.h"

namespace circa {

// This file is deprecated, functionality should be moved to cpp_convenience.h

struct StringList {

    Branch data;

    void append(std::string const& str) {
        create_string(data, str);
    }

    std::string const& operator[](int index) {
        return as_string(data[index]);
    }

    int length() {
        return data.length();
    }
};

}
