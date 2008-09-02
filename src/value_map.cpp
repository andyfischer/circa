
#include "common_headers.h"

#include "bootstrapping.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "operations.h"
#include "type.h"
#include "value_map.h"

namespace circa {

Term*
ValueMap::findValueForKey(Term* term)
{
    PairList::iterator it;
    for (it = pairs.begin(); it != pairs.end(); ++it) {
        if (term->equals(it->key))
            return it->value;
    }
    return NULL;
}

void
ValueMap::set(Term* key, Term* value)
{
    Term* existingValue = findValueForKey(key);

    if (existingValue == NULL) {
        Pair newPair;
        newPair.key = create_constant(&branch, key->type);
        duplicate_value(key, newPair.key);
        newPair.value = create_constant(&branch, value->type);
        duplicate_value(value, newPair.value);
        pairs.push_back(newPair);
    } else {
        duplicate_value(value, existingValue);
    }
}

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

    Term* value = map.findValueForKey(term->inputs[1]);
    
    change_type(term, value->type);
    duplicate_value(value, term);
}

void map_set__evaluate(Term* term)
{
    Term* key = term->inputs[1];
    Term* value = term->inputs[2];

    recycle_value(term->inputs[0], term);

    as<ValueMap>(term).set(key, value);
}

void initialize_map_function(Branch* kernel)
{
    MAP_TYPE = quick_create_cpp_type<ValueMap>(kernel, "Map");

    Term* accessFunc = quick_create_function(kernel, "map-access", map_access__evaluate,
        TermList(MAP_TYPE, ANY_TYPE), ANY_TYPE);

    Term* setFunc = quick_create_function(kernel, "map-set", map_set__evaluate,
        TermList(MAP_TYPE, ANY_TYPE, ANY_TYPE), MAP_TYPE);
    
    as_type(MAP_TYPE)->addMemberFunction("", accessFunc);
}

} // namespace circa
