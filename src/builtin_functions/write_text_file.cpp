// Copyright 2008 Andrew Fischer

namespace write_text_file_function {

    void evaluate(Term* caller)
    {
        std::string filename = as_string(caller->inputs[0]);
        std::string contents = as_string(caller->inputs[1]);
        std::ofstream file;
        file.open(filename.c_str(), std::ios::out);
        file << contents;
        file.close();
    }

    void setup(Branch& kernel)
    {
        import_c_function(kernel, evaluate,
                "function write-text-file(string, string)");
    }
}
