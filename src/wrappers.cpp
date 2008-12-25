// Copyright 2008 Andrew Fischer

#include "ast.h"
#include "branch.h"
#include "cpp_interface.h"
#include "values.h"

namespace circa {

std::string getInfixFunctionName(std::string infix)
{
    Branch workspace;
    string_value(workspace, infix, "infix");
    return eval_as<std::string>(workspace, "get-infix-function-name(infix)");
}
    
} // namespace circa
