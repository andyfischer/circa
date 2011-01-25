// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "timer.h"

namespace circa {

std::ostream &operator<<(std::ostream &stream, Timer timer)
{
    stream << timer.reset() << " ms";
    return stream;
}

} // namespace circa
