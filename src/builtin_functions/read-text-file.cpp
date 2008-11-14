// Copyright 2008 Andrew Fischer

namespace read_text_file_function {

    void evaluate(Term* caller)
    {
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
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function read-text-file(string) -> string");
        as_function(main_func).pureFunction = false;
    }
}
