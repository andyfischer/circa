// Copyright 2008 Andrew Fischer

#ifndef CIRCA_WRAPPERS_INCLUDED
#define CIRCA_WRAPPERS_INCLUDED

/*
 * This file contains wrappers for Circa functions, so that they can
 * be easily called from C++ code.
*/

namespace circa {

std::string getInfixFunctionName(std::string infix);
std::string read_text_file(std::string const& filename);

std::string eval_as_string(Term* function, Term* input0);

} // namespace circa

#endif
