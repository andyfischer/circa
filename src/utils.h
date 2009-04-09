// Copyright 2008 Paul Hodge

#ifndef CIRCA_UTILS_INCLUDED
#define CIRCA_UTILS_INCLUDED

// This file contains miscellaneous utility functions which do not belong
// anywhere else. Functions should only be added here if there really is
// no better place for them.

namespace circa {

std::string read_text_file(std::string const& filename);
void write_text_file(std::string const& filename, std::string const& contents);

} // namespace circa

#endif
