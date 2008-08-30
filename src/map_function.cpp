
#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "map_function.h"

namespace circa {

void map__create__evaluate(Term* term)
{

}

void initialize_map_function(Branch* kernel)
{
    quick_create_function(kernel, "map", map__create__evaluate,
        TermList(TYPE_TYPE, TYPE_TYPE), FUNCTION_TYPE);
}

} // namespace circa
