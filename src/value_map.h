#ifndef CIRCA__MAP_FUNCTION__INCLUDED
#define CIRCA__MAP_FUNCTION__INCLUDED

#include "branch.h"
#include "term_map.h"

namespace circa {

struct ValueMap
{
    struct Pair {
        Term* key;
        Term* value;
    };
    typedef std::vector<Pair> PairList;

    Branch branch;
    PairList pairs;

    Term* findValueForKey(Term* term);
    void set(Term* key, Term* value);
};

void initialize_map_function(Branch* kernel);

} // namespace circa

#endif
