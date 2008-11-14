
// Copyright 2008 Paul Hodge

namespace list_apply_function {

    void evaluate(Term* caller)
    {
        as_function(caller->inputs[0]);
        List& list = as_list(caller->inputs[1]);
        
        as_list(caller).clear();
        
        for (int i=0; i < list.count(); i++) {
            Term* result = apply_function(*caller->owningBranch, caller->inputs[0], ReferenceList(list.get(i)));
        
            evaluate_term(result);
        
            as_list(caller).append(result);
        }
    }


    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function list-apply(Function, List) -> List");
        as_function(main_func).pureFunction = true;


    }
}
