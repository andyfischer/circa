// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace cpp_importing_tests {

class Type1 {
public:
    static int gInstanceCount;

    std::string myString;

    Type1() {
        gInstanceCount++;
    }
    ~Type1() {
        gInstanceCount--;
    }
};

int Type1::gInstanceCount = 0;

void append_string__evaluate(Term* term)
{
    assign_value(term->input(0), term);
    as<Type1>(term).myString += as_string(term->input(1));
}

void default_function() {
    /* TODO
    Branch branch;

    Term* type1 = import_type<Type1>(branch, "Type1");

    Term* appendStr = import_function(branch, append_string__evaluate,
            "append_str(Type1, string) -> Type1");

    as_type(type1).addMemberFunction(appendStr, "");

    Term* hi = branch.eval("hi = Type1()");

    as<Type1>(hi).myString = "hi";

    Term* hi2u = branch.eval("hi2u = hi(\"2u\"))");

    test_assert(as<Type1>(hi2u).myString == "hi2u");
    */
}

void register_tests()
{
    REGISTER_TEST_CASE(cpp_importing_tests::default_function);
}

} // namespace cpp_importing_tests

} // namespace circa
