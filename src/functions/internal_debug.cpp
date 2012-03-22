// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace internal_debug_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(dump_parse, "dump_parse(any...)"
        "'For internal debugging. The parser will dump information about all input terms"
        "immediately after this function is parsed")
    {
    }

    void dump_parse_post_compile(Term* term)
    {
        std::cout << "dump_parse " << global_id(term) << ": ";
        for (int i=0; i < term->numInputs(); i++) {
            if (i != 0) std::cout << ", ";
            print_term(std::cout, term->input(0));
        }
        std::cout << std::endl;
    }

    List oraclecaValues;

    CA_DEFINE_FUNCTION(oracle, "test_oracle() -> any"
        "'For internal testing. This function will output values that are manually "
        "inserted with the c++ function oracle_send'")
    {
        if (oraclecaValues.length() == 0)
            set_null(OUTPUT);
        else {
            copy(oraclecaValues[0], OUTPUT);
            oraclecaValues.remove(0);
        }
    }

    void oracle_clear()
    {
        oraclecaValues.clear();
    }

    void oracle_send(caValue* value)
    {
        copy(value, oraclecaValues.append());
    }

    void oracle_send(int i)
    {
        Value v;
        set_int(&v, i);
        oracle_send(&v);
    }

    List spycaValues;

    CA_DEFINE_FUNCTION(spy, "test_spy(any)"
            "'For internal testing. This function will save every inputs to a static list, "
            "and the contents of this list can be checked from C++ code.")
    {
        copy(INPUT(0), spycaValues.append());
    }

    void spy_clear()
    {
        spycaValues.clear();
    }
    List* spy_results()
    {
        return &spycaValues;
    }

    bool g_initializedHandleType;
    Type g_testHandleType;
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

    CA_DEFINE_FUNCTION(get_term_stack, "debug_get_term_stack() -> List")
    {
        int len = CONTEXT->numFrames;
        List& output = *List::cast(OUTPUT, len);
        for (int i=0; i < len; i++)
            set_ref(output[i], get_frame_from_bottom(CONTEXT, i)->branch->owningTerm);
    }

    CA_DEFINE_FUNCTION(dump_current_branch, "dump_current_branch()")
    {
        dump(*CALLER->owningBranch);
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        as_function(kernel->get("dump_parse"))->postCompile = dump_parse_post_compile;
    }
}
}
