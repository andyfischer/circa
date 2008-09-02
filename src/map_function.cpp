
#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "map_function.h"
#include "type.h"

namespace circa {

/*
void map_create__execute(Term* term)
{
    Term* inputType = term->inputs[0];
    Term* outputType = term->inputs[1];

    Function *function = as_function(term);
    function->inputTypes = TermList(inputType);
    function->outputType = outputType;
    function->name = "map<" + as_type(inputType)->name + "," + as_type(outputType)->name + ">";
    function->execute = map__execute;
}
*/

void map_access__evaluate(Term* term)
{
    ValueMap& map = as<ValueMap>(term->inputs[0]);

}

void initialize_map_function(Branch* kernel)
{
    MAP_TYPE = quick_create_cpp_type<ValueMap>(kernel, "Map");

    Term* accessFunc = quick_create_function(kernel, "map-access", map_access__evaluate,
        TermList(MAP_TYPE, ANY_TYPE), ANY_TYPE);
    
    as_type(MAP_TYPE)->addMemberFunction("", accessFunc);
}

} // namespace circa
