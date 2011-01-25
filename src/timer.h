// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <ctime>
#include <iostream>

// A simple timer class. This is based on clock(), so it's low-resolution.

namespace circa {

class Timer {
public:

    clock_t _startTime;

    Timer()
    {
        _startTime = clock();
    }

    // Advance our start time and return elapsed time.
    clock_t reset()
    {
        clock_t current = clock();

        clock_t elapsed = (current - _startTime) * 1000 / CLOCKS_PER_SEC;

        _startTime += elapsed;

        return elapsed;
    }

    friend std::ostream &operator<<(std::ostream &stream, Timer timer);
};

std::ostream &operator<<(std::ostream &stream, Timer timer);

} // namespace circa
