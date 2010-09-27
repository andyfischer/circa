// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

struct InputInfo
{
    int relativeScope;

    InputInfo()
      : relativeScope(0)
    {}

    std::string toShortString();
};

} // namespace circa
