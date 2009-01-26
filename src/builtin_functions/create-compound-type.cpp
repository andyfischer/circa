// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace create_compound_type_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->input(0));
        Type& output = as_type(caller);

        output.name = name;
        output.alloc = CompoundValue::alloc;
        output.dealloc = CompoundValue::dealloc;
        output.startPointerIterator = CompoundValue::start_pointer_iterator;
    }

    void setup(Branch& kernel)
    {
        Term* main = import_function(kernel, evaluate, "create-compound-type(string) -> Type");
        as_function(main).pureFunction = true;
    }
}
} // namespace circa
