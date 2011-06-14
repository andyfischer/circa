// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

namespace circa {
namespace dynamic_call_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(dynamic_call, "dynamic_call(Function f, List args)")
    {
        std::cout << INPUT(0)->toString() << std::endl;
        std::cout << INPUT(1)->toString() << std::endl;
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
