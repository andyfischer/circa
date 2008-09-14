// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "tests/common.h"
#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "type.h"

namespace circa {
namespace type_tests {

void derived_type()
{
    Branch branch;

    Term* type1 = create_constant(&branch, TYPE_TYPE);
    as_type(type1)->dataSize = 6;

    test_assert(as_type(type1)->getInstanceOffset() == 0);

    Term* derivedType = create_constant(&branch, TYPE_TYPE);
    as_type(derivedType)->dataSize = 12;
    as_type(derivedType)->parentType = type1;
    test_assert(as_type(derivedType)->getInstanceOffset() == 6);

    Term* derivedType2 = create_constant(&branch, TYPE_TYPE);
    as_type(derivedType2)->dataSize = 3;
    as_type(derivedType2)->parentType = derivedType;
    test_assert(as_type(derivedType2)->getInstanceOffset() == 18);
}

void test_fields()
{
    Branch branch;
    Term* mytype = create_empty_type(&branch);
    as_type(mytype)->fields["field-name"] = INT_TYPE;

    Term* myinst = create_constant(&branch, mytype);

    test_assert(myinst->fields["field-name"]->type == INT_TYPE);
}


} // namespace type_tests

void register_type_tests()
{
    REGISTER_TEST_CASE(type_tests::derived_type);
    REGISTER_TEST_CASE(type_tests::test_fields);
}

} // namespace circa
