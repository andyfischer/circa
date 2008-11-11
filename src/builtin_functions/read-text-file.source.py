# Copyright 2008 Paul Hodge

header = "function read-text-file(string) -> string"
pure = False
evaluate = r"""
        std::string filename = as_string(caller->inputs[0]);
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
        as_string(caller) = contents.str();
"""
