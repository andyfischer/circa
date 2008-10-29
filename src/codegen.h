// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CODEGEN_INCLUDED
#define CIRCA_CODEGEN_INCLUDED

#include "common_headers.h"

#include "output_stream.h"

namespace circa {

void generate_cpp_code(Term* term, OutputStream& output);
void initialize_codegen_functions(Branch& kernel);

}

#endif
