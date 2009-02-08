// Copyright 2008 Paul Hodge

#include "branch.h"
#include "builtins.h"
#include "cpp_importing.h"
#include "importing.h"
#include "newparser.h"
#include "runtime.h"
#include "term.h"
#include "token_stream.h"
#include "type.h"
#include "values.h"

namespace circa {

Term* import_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    Term* result = newparser::compile(branch, newparser::function_from_header, headerText);

    as_function(result).evaluate = evaluate;
    return result;
}

Term* import_member_function(Term* type, Function::EvaluateFunc evaluate, std::string const& headerText)
{
    static Branch* temporaryBranchBecauseNewParserNeedsARealBranch = NULL;

    if (temporaryBranchBecauseNewParserNeedsARealBranch == NULL)
        temporaryBranchBecauseNewParserNeedsARealBranch = new Branch();

    Term* result = newparser::compile(*temporaryBranchBecauseNewParserNeedsARealBranch,
            newparser::function_from_header, headerText);

    as_function(result).evaluate = evaluate;
    as_type(type).addMemberFunction(result, as_function(result).name);
    return result;
}

} // namespace circa
