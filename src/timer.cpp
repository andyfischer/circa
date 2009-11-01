// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "timer.h"

namespace circa {

std::ostream &operator<<(std::ostream &stream, Timer timer)
{
    stream << timer.reset() << " ms";
    return stream;
}

} // namespace circa
