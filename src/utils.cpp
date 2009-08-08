// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

std::string read_text_file(std::string const& filename)
{
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);
    std::stringstream contents;
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (!firstLine)
            contents << "\n";
        contents << line;
        firstLine = false;
    }
    file.close();
    return contents.str();
}

void write_text_file(std::string const& filename, std::string const& contents)
{
    std::ofstream file;
    file.open(filename.c_str(), std::ios::out);
    file << contents;
    file.close();
}

std::string get_directory_for_filename(std::string const& filename)
{
    // Really should use boost::path or some other library here.
    size_t last_slash = filename.find_last_of("/");

    if (last_slash == filename.npos)
        return ".";

    if (last_slash == 0)
        return "/";

    std::string result = filename.substr(0, last_slash);

    return result;
}
    
} // namespace circa
