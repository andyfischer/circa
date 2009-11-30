// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace cpp_importing_tests {

class Type1 {
public:
    static int gInstanceCount;

    std::string myString;

    Type1() {
        gInstanceCount++;
    }
    ~Type1() {
        gInstanceCount--;
    }
};

int Type1::gInstanceCount = 0;

void register_tests()
{
    // TODO
}

} // namespace cpp_importing_tests

} // namespace circa
