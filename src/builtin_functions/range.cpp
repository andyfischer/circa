// Copyright 2008 Paul Hodge

namespace range_function {

    void evaluate(Term* caller)
    {
        unsigned int max = as_int(caller->inputs[0]);
        
        as_list(caller).clear();
        
        for (unsigned int i=0; i < max; i++) {
            as_list(caller).append(int_var(*caller->owningBranch, i));
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function range(int) -> List");
        as_function(main_func).pureFunction = true;
    }
}
