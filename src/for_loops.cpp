// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

Term* get_for_loop_iterator(Term* loopTerm)
{
    std::string name = as_branch(loopTerm->state)["iteratorName"]->asString();
    return loopTerm->state->asBranch()["contents"]->asBranch()[name];
}

} // namespace circa
