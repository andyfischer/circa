// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

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
    }

    List oracleTaggedValues;

    CA_DEFINE_FUNCTION(oracle, "test_oracle() -> any"
        "'For internal testing. This function will output values that are manually "
        "inserted with the c++ function oracle_send'")
    {
        if (oracleTaggedValues.length() == 0)
            set_null(OUTPUT);
        else {
            copy(oracleTaggedValues[0], OUTPUT);
            oracleTaggedValues.remove(0);
        }
    }

    void oracle_clear()
    {
        oracleTaggedValues.clear();
    }

    void oracle_send(TaggedValue* value)
    {
        copy(value, oracleTaggedValues.append());
    }

    void oracle_send(int i)
    {
        TaggedValue v;
        set_int(&v, i);
        oracle_send(&v);
    }

    List spyTaggedValues;

    CA_DEFINE_FUNCTION(spy, "test_spy(any)"
            "'For internal testing. This function will save every inputs to a static list, "
            "and the contents of this list can be checked from C++ code.")
    {
        copy(INPUT(0), spyTaggedValues.append());
    }

    void spy_clear()
    {
        spyTaggedValues.clear();
    }
    List* spy_results()
    {
        return &spyTaggedValues;
    }

    CA_DEFINE_FUNCTION(dump_scope_state, "dump_scope_state() -> any")
    {
        copy(&CONTEXT->currentScopeState, OUTPUT);
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
        int len = CONTEXT->callStack.length();
        List& output = *List::cast(OUTPUT, len);
        for (int i=0; i < len; i++)
            set_ref(output[i], CONTEXT->callStack[i]);
    }

    CA_DEFINE_FUNCTION(dump_current_branch, "dump_current_branch()")
    {
        dump(*CALLER->owningBranch);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        get_function_attrs(kernel["dump_parse"])->postCompile = dump_parse_post_compile;
    }
}
}
