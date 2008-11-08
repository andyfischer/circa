# Copyright 2008 Paul Hodge

header = "function write-text-file(string, string)"
pure = False
evaluate = """
        std::string filename = as_string(caller->inputs[0]);
        std::string contents = as_string(caller->inputs[1]);
        std::ofstream file;
        file.open(filename.c_str(), std::ios::out);
        file << contents;
        file.close();
"""
