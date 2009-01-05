// Copyright 2008 Paul Hodge

#include "circa.h"
#include "compilation.h"

namespace circa {
namespace evaluate_file_function {

    void evaluate(Term* caller)
    {
        std::string &filename = as_string(caller->input(0));
        Branch &output = as_branch(caller);

        Branch workspace;
        workspace.bindName(string_value(workspace, filename), "filename");
        std::string file_contents = as_string(workspace.eval(
                    "read-text-file(filename)"));

        token_stream::TokenStream tokens(file_contents);
        ast::StatementList *statementList = parser::statementList(tokens);

        CompilationContext context;
        context.pushScope(&output, NULL);
        statementList->createTerms(context);
        context.popScope();

        delete statementList;
    }

    void setup(Branch& kernel)
    {
        /*Term* main_func = */import_c_function(kernel, evaluate,
                "function evaluate-file(string) -> Branch");
    }
}
} // namespace circa
