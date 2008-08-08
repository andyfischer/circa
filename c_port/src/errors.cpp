
#include "common_headers.h"

#include "errors.h"
#include "term.h"
#include "type.h"

namespace circa {

namespace errors {

string
TypeError::message()
{
    // Usually we don't check that term->type is a type, but we do it
    // here in TypeError to avoid infinitely throwing exceptions.
    if (!is_type(term->type))
        throw InternalError("term->type is not a type");

    if (!is_type(expectedType))
        throw InternalError("2nd argument to TypeError must be a type");

    return string("TypeError: expected " + as_type(expectedType)->name
            + ", found " + as_type(term->type)->name + " \""
            + term->findName()) + "\"";
}

} // namespace errors
} // namespace circa
