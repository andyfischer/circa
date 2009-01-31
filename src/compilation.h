// Copyright 2008 Andrew Fischer

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

namespace circa {

namespace ast {
    class FunctionCall;
    class Infix;
    class LiteralString;
    class LiteralFloat;
    class LiteralInteger;
    class StatefulValueDeclaration;
}

struct CompilationContext
{
    struct Scope {
        Branch* branch;
        Term* branchOwner;

        Scope() : branch(NULL), branchOwner(NULL) {}
        Scope(Branch* _b, Term* _bo) : branch(_b), branchOwner(_bo) {}
    };
    typedef std::vector<Scope> ScopeList;

    struct ExpressionFrame {
        bool insideExpression;

        ExpressionFrame() : insideExpression(false) {}
        ExpressionFrame(bool _ie) : insideExpression(_ie) {}
    };
    typedef std::vector<ExpressionFrame> ExpressionFrameList;

    ScopeList scopeStack;
    ExpressionFrameList expressionStack;
    std::string pendingRebind;

    CompilationContext() {}
    CompilationContext(Branch* root) { pushScope(root, NULL); }

    Term* findNamed(std::string const& name) const;

    Scope const& topScope() const;

    Branch& topBranch() const;

    void pushScope(Branch* branch, Term* owningTerm);
    void popScope();
};

Term* find_and_apply_function(Branch& branch,
        std::string const& functionName,
        ReferenceList inputs);

bool push_is_inside_expression(Branch& branch, bool value);
void pop_is_inside_expression(Branch& branch, bool value);
void remove_compilation_attrs(Branch& branch);

Term* create_comment(Branch& branch, std::string const& text);
Term* create_literal_string(Branch& branch, ast::LiteralString& ast);
Term* create_literal_float(Branch& branch, ast::LiteralFloat& ast);
Term* create_literal_integer(Branch& branch, ast::LiteralInteger& ast);
Term* create_dot_concatenated_call(CompilationContext &context, ast::Infix& ast);
Term* create_arrow_concatenated_call(CompilationContext &context, ast::Infix& ast);
Term* create_feedback_call(CompilationContext &context, ast::Infix& ast);
Term* create_infix_call(CompilationContext &context, ast::Infix& ast);
Term* create_function_call(CompilationContext &context, ast::FunctionCall& ast);
Term* create_stateful_value_declaration(CompilationContext &context,
        ast::StatefulValueDeclaration& ast);

} // namespace circa

#endif
