// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Term* import_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& header)
{
    Term* result = create_empty_function(branch, header);
    as_function(result).evaluate = evaluate;
    return result;
}

Term* import_member_function(Term* type, Function::EvaluateFunc evaluate, std::string const& header)
{
    static Branch* temporaryBranchBecauseNewParserNeedsARealBranch = NULL;

    if (temporaryBranchBecauseNewParserNeedsARealBranch == NULL)
        temporaryBranchBecauseNewParserNeedsARealBranch = new Branch();

    Term* result = parser::compile(*temporaryBranchBecauseNewParserNeedsARealBranch,
            parser::function_from_header, header);

    as_function(result).evaluate = evaluate;
    as_type(type).addMemberFunction(result, as_function(result).name);
    return result;
}

} // namespace circa
