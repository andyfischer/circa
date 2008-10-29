// Copyright 2008 Andrew Fischer

namespace to_string_function {

    void evaluate(Term* caller) {
        Term* term = caller->inputs[0];

        Type::ToStringFunc func = as_type(term->type).toString;

        if (func == NULL) {
            as_string(caller) = std::string("<" + as_type(term->type).name
                    + " " + term->name + ">");
        } else {
            as_string(caller) = func(term);
        }
    }
    void setup(Branch& kernel)
    {
        import_c_function(kernel, evaluate,
            "function to-string(any) -> string");
    }
}
