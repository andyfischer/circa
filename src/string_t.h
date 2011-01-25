// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {
namespace string_t {

    struct StringData;

}

// C++ wrapper over string_T
class String
{
    string_t::StringData* _data;

    String() : _data(NULL) {}
    ~String();
};

} // namespace circa
