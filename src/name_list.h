// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NameList
{
    std::vector<std::string> names;

    int count() const { return (int) names.size(); }
    const char* operator[](int index) const { return names[index].c_str(); }
};

} // namespace circa
