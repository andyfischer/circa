// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "Common.h"

void Log(const char* msg)
{
    printf("%s\n", msg);
}

void Log(const char* msg, caValue* arg1)
{
    circa::Value arg1str;
    circa_to_string_repr(arg1, &arg1str);
    printf("%s%s\n", msg, circa_string(&arg1str));
}
void Log(const char* msg, const char* arg1)
{
    printf("%s%s\n", msg, arg1);
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
