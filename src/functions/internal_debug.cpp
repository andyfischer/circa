// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

#include "types/simple_handle.h"

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

    List oracleValues;

    CA_DEFINE_FUNCTION(oracle, "test_oracle() -> any"
        "'For internal testing. This function will output values that are manually "
        "inserted with the c++ function oracle_send'")
    {
        if (oracleValues.length() == 0)
            set_null(OUTPUT);
        else {
            copy(oracleValues[0], OUTPUT);
            oracleValues.remove(0);
        }
    }

    void oracle_clear()
    {
        oracleValues.clear();
    }

    void oracle_send(TaggedValue* value)
    {
        copy(value, oracleValues.append());
    }

    void oracle_send(int i)
    {
        TaggedValue v;
        set_int(&v, i);
        oracle_send(&v);
    }

    List spyValues;

    CA_DEFINE_FUNCTION(spy, "test_spy(any)"
            "'For internal testing. This function will save every inputs to a static list, "
            "and the contents of this list can be checked from C++ code.")
    {
        copy(INPUT(0), spyValues.append());
    }

    void spy_clear()
    {
        spyValues.clear();
    }
    List* spy_results()
    {
        return &spyValues;
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

    CA_DEFINE_FUNCTION(alloc_handle, "debug_alloc_handle(any handle) -> any")
    {
        if (!g_initializedHandleType) {
            simple_handle_t::setup_type(&g_testHandleType);
            set_opaque_pointer(&g_testHandleType.parameter,
                    (void*) test_handle_on_release);
            g_testHandleType.name = "TestHandle";
            memset(g_testHandleAllocated, sizeof(g_testHandleAllocated), 0);
            g_initializedHandleType = true;
        }

        if (INPUT(0)->value_type == &g_testHandleType) {
            copy(INPUT(0), OUTPUT);
            return;
        }

        int slot = test_handle_find_free_slot();
        simple_handle_t::set(&g_testHandleType, OUTPUT, slot);
        //std::cout << "allocated " << slot << std::endl;
    }

    CA_DEFINE_FUNCTION(get_term_stack, "debug_get_term_stack() -> List")
    {
        int len = CONTEXT->stack.length();
        List& output = *List::cast(OUTPUT, len);
        for (int i=0; i < len; i++)
            set_ref(output[i], CONTEXT->stack[i]);
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
