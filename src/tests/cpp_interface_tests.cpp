// Copyright 2008 Andrew Fischer

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
    recycle_value(term->input(0), term);
    as<Type1>(term).myString += as_string(term->input(1));
}

void default_function() {
    Branch branch;

    Term* type1 = import_type<Type1>(branch, "Type1");

    Term* appendStr = import_function(branch, append_string__evaluate,
            "append-str(Type1, string) -> Type1");

    as_type(type1).addMemberFunction("", appendStr);

    Term* hi = branch.eval("hi = Type1()");

    as<Type1>(hi).myString = "hi";

    Term* hi2u = branch.eval("hi2u = hi(\"2u\"))");

    test_assert(as<Type1>(hi2u).myString == "hi2u");
}

} // namespace cpp_interface_tests

void register_cpp_interface_tests()
{
    REGISTER_TEST_CASE(cpp_interface_tests::default_function);
}

} // namespace circa
