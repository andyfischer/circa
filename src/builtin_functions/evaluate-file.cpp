// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "compilation.h"

namespace circa {
namespace evaluate_file_function {

    void evaluate(Term* caller)
    {
        std::string &filename = as_string(caller->input(0));
        Branch &output = as_branch(caller);

        Branch workspace;
        workspace.bindName(string_var(workspace, filename), "filename");
        std::string file_contents = as_string(eval_statement(workspace,
                    "read-text-file(filename)"));

        token_stream::TokenStream tokens(file_contents);
        ast::StatementList *statementList = parser::statementList(tokens);

        CompilationContext context;
        context.push(&output, NULL);
        statementList->createTerms(context);
        context.pop();

        delete statementList;
    }

    void setup(Branch& kernel)
    {
        /*Term* main_func = */import_c_function(kernel, evaluate,
                "function evaluate-file(string) -> Branch");
    }
}
} // namespace circa
