// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TIMER_INCLUDED
#define CIRCA_TIMER_INCLUDED

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

#endif
