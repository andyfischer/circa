// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef CIRCA_UTILS_INCLUDED
#define CIRCA_UTILS_INCLUDED

// This file contains miscellaneous utility functions which do not belong
// anywhere else. Functions should only be added here if there really is
// no better place for them.

namespace circa {

std::string read_text_file(std::string const& filename);
void write_text_file(std::string const& filename, std::string const& contents);
std::string get_directory_for_filename(std::string const& filename);

} // namespace circa

#endif
