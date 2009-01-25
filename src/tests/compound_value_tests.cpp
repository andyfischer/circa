// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace compound_value_tests {

void test_iterator()
{
    CompoundValue value;

    Term* ref = value.appendSlot(REFERENCE_TYPE);
    
    as_ref(ref) = INT_TYPE;

    PointerIterator* it = start_compound_value_pointer_iterator(&value);

    test_assert(it->current()->function == VALUE_FUNCTION_GENERATOR);
    it->advance();
    test_assert(it->current() == REFERENCE_TYPE);
    it->advance();
    test_assert(it->current() == INT_TYPE);
    it->advance();
    test_assert(it->finished());

    delete it;
}

void register_tests()
{
    REGISTER_TEST_CASE(compound_value_tests::test_iterator);
}

} // namespace compound_value_tests
} // namespace circa
