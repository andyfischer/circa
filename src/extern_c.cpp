// Copyright 2008 Paul Hodge

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
