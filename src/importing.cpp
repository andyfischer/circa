
#include "branch.h"
#include "builtins.h"
#include "importing.h"
#include "operations.h"
#include "term.h"

namespace circa {

Term* import_c_function(Branch* branch, Function::ExecuteFunc execute, std::string const& header)
{
    Term* term = create_constant(branch, FUNCTION_TYPE);
    Function* func = as_function(term);

    // todo
    return NULL;
}

}
