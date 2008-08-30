#ifndef CIRCA__MAP_FUNCTION__INCLUDED
#define CIRCA__MAP_FUNCTION__INCLUDED

#include "branch.h"
#include "function.h"

namespace circa {

struct MapFunctionInstance : public Function
{
    Branch branch;
    TermMap map;
};

void initialize_map_function(Branch* kernel);

} // namespace circa

#endif
