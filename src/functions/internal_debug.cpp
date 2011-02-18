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
        std::cout << "dump_parse " << format_global_id(term) << ": ";
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
        ca_assert(oracleValues.length() > 0);
        copy(oracleValues[0], OUTPUT);
        oracleValues.remove(0);
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

    CA_DEFINE_FUNCTION(spy, "debug_spy(any)"
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

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        get_function_attrs(kernel["dump_parse"])->postCompile = dump_parse_post_compile;
    }
}
}
