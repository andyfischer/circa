// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"
#include "input_info.h"
#include "tagged_value.h"
#include "types/list.h"

namespace circa {

std::string
InputInfo::toShortString()
{
    std::stringstream out;
    out << "-" << relativeScope;
    return out.str();
}

} // namespace circa
