
// Copyright 2008 Paul Hodge

namespace print_function {

    void evaluate(Term* caller)
    {
        std::cout << as_string(caller->inputs[0]) << std::endl;
    }


    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function print(string)");
        as_function(main_func).pureFunction = false;


    }
}
