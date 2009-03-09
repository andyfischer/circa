// Copyright 2008 Andrew Fischer

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
    
} // namespace circa
