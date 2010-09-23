// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include <vector>

#include "input_info.h"

namespace circa {

struct InputInfoList
{
    std::vector<InputInfo> list;

    void resize(int size) { list.resize(size); }
    int length() { return int(list.size()); }
    InputInfo& operator[](int index) { return list[index]; }
};

} // namespace circa
