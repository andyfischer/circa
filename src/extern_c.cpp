// Copyright 2008 Andrew Fischer

#include "circa.h"

using namespace circa;

extern "C" {

Branch* new_branch()
{
    return new Branch();
}

void evaluate_file(Branch* branch, const char* filename)
{
    evaluate_file(*branch, filename);
}

}
