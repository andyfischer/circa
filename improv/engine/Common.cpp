// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

void Log(const char* fmt, ...)
{
    printf("%s", fmt);
}

int NextPowerOfTwo(int i)
{
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return i + 1;
}
