
// Copyright 2008 Andrew Fischer

namespace concat_function {

    void evaluate(Term* caller)
    {
        std::stringstream out;
        for (unsigned int index=0; index < caller->inputs.count(); index++) {
            out << as_string(caller->inputs[index]);
        }
        as_string(caller) = out.str();
    }


    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function concat(string,string) -> string");
        as_function(main_func).pureFunction = true;


    }
}
