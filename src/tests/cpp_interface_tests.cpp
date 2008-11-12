// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "testing.h"
#include "cpp_interface.h"
#include "branch.h"
#include "builtins.h"
#include "parser.h"
#include "values.h"

namespace circa {
namespace cpp_interface_tests {

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
    recycle_value(term->inputs[0], term);
    as<Type1>(term).myString += as_string(term->inputs[1]);
}

void memory_management() {
    Branch branch;

    Type1::gInstanceCount = 0;
    quick_create_cpp_type<Type1>(&branch, "Type1");

    test_assert(Type1::gInstanceCount == 0);
    Term* term = apply_statement(branch, "a = Type1()");
    test_assert(Type1::gInstanceCount == 1);

    delete term;

    test_assert(Type1::gInstanceCount == 0);
}

void default_function() {
    Branch branch;

    Term* type1 = quick_create_cpp_type<Type1>(&branch, "Type1");

    Term* appendStr = quick_create_function(&branch, "append-str",
            append_string__evaluate, ReferenceList(type1, STRING_TYPE), type1);

    as_type(type1).addMemberFunction("", appendStr);

    Term* hi = eval_statement(branch, "hi = Type1()");

    as<Type1>(hi).myString = "hi";

    Term* hi2u = eval_statement(branch, "hi2u = hi(\"2u\"))");

    test_assert(as<Type1>(hi2u).myString == "hi2u");
}

} // namespace cpp_interface_tests

void register_cpp_interface_tests()
{
    REGISTER_TEST_CASE(cpp_interface_tests::memory_management);
    REGISTER_TEST_CASE(cpp_interface_tests::default_function);
}

} // namespace circa
