
#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "map_function.h"

namespace circa {

void map__evaluate(Term* term);

struct MapFunctionState
{
    Branch branch;
    TermMap map;
};

Term* MAP_FUNCTION_STATE_TYPE = NULL;

void map_create__evaluate(Term* term)
{
    Term* inputType = term->inputs[0];
    Term* outputType = term->inputs[1];

    Function *function = as_function(term);
    function->inputTypes = TermList(inputType);
    function->outputType = outputType;
    function->name = "map<" + as_type(inputType)->name + "," + as_type(outputType)->name + ">";
}

void map__evaluate(Term* term)
{

}

void initialize_map_function(Branch* kernel)
{
    MAP_FUNCTION_STATE_TYPE = quick_create_cpp_type<MapFunctionState>(kernel, "MapState");
    quick_create_function(kernel, "map", map_create__evaluate,
        TermList(TYPE_TYPE, TYPE_TYPE), FUNCTION_TYPE);
}

} // namespace circa
