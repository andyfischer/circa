
#include "CommonHeaders.h"

#include "Errors.h"

void INTERNAL_ERROR(const string msg)
{
    printf("ERROR: ");
    printf(msg.c_str());
    printf("\n");
    throw std::exception();
}

