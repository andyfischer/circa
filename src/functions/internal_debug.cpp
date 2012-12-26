// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace internal_debug_function {

    void dump_parse_post_compile(Term* term)
    {
        std::cout << "dump_parse " << global_id(term) << ": ";
        for (int i=0; i < term->numInputs(); i++) {
            if (i != 0) std::cout << ", ";
            print_term(term->input(0), std::cout);
        }
        std::cout << std::endl;
    }


    bool g_initializedHandleType;
    const int g_testHandleSlots = 100;
    bool g_testHandleAllocated[g_testHandleSlots];

    int test_handle_find_free_slot()
    {
        for (int i=0; i < g_testHandleSlots; i++)
            if (!g_testHandleAllocated[i])
                return i;
        ca_assert(false);
        return 0;
    }

    void test_handle_on_release(int handle)
    {
        ca_assert(g_testHandleAllocated[handle]);
        g_testHandleAllocated[handle] = false;
        //std::cout << "released " << handle << std::endl;
    }

    void dump_current_block(caStack* stack)
    {
        Term* caller = (Term*) circa_caller_term(stack);
        dump(caller->owningBlock);
    }

    void setup(Block* kernel)
    {
        import_function(kernel, NULL, "dump_parse(any t :multiple)\n"
            " -- For internal debugging. The parser will dump information about all input terms\n"
            " -- immediately after this function is parsed");
        as_function(kernel->get("dump_parse"))->postCompile = dump_parse_post_compile;

        import_function(kernel, dump_current_block, "dump_current_block()");
    }
}
} // namespace circa
