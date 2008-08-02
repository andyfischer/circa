
#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "term.h"
#include "type.h"

namespace circa {

Type::Type()
  : name("undefined"),
    alloc(NULL),
    copy(NULL),
    remapPointers(NULL),
    toString(NULL)
{
}

bool is_type(Term* term)
{
    return ((term->type == BUILTIN_TYPE_TYPE)
            || (term->type == BUILTIN_STRUCT_DEFINITION_TYPE));
}

Type* as_type(Term* term)
{
    if (!is_type(term))
        throw errors::TypeError(term, BUILTIN_TYPE_TYPE);

    return (Type*) term->value;
}

void Type_alloc(Term* caller)
{
    caller->value = new Type();
}

void set_member_function(Term* type, std::string name, Term* function)
{
    Type* typeData = as_type(type);
    as_function(function);

    typeData->memberFunctions.bind(function, name);
}

Term* get_member_function(Term* type, std::string name)
{
    return as_type(type)->memberFunctions[name];
}

} // namespace circa
