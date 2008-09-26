// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "errors.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace errors {

string
TypeError::message()
{
    if (expectedType == NULL)
        throw InternalError("In TypeError: expectedType is NULL");

    if (!is_type(expectedType))
        throw InternalError("In TypeError: expectedType must be a type");

    std::string expected = as_type(expectedType)->name;

    if (offendingTerm == NULL)
        return std::string("In TypeError: Term is NULL (expected ") + expected + ")";

    if (offendingTerm->type == NULL)
        return std::string("Term '") + offendingTerm->findName() + "' has NULL type "
            "(expected " + expected + ")";

    if (!is_type(offendingTerm->type))
        return std::string("The type field of '") + offendingTerm->findName()
            + "' is not a type. (expected " + expected + ")";

    return std::string("TypeError: expected " + as_type(expectedType)->name
            + ", found " + as_type(offendingTerm->type)->name)
            + " = " + offendingTerm->toString();
}

} // namespace errors
} // namespace circa
