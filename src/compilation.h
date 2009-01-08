// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

namespace circa {

namespace ast {
    class FunctionCall;
    class Infix;
    class LiteralString;
    class LiteralFloat;
    class LiteralInteger;
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

        ExpressionFrame() : insideExpression(true) {}
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

    void pushExpressionFrame(bool insideExpression);
    void popExpressionFrame();

    bool isInsideExpression() const;
};

Term* find_and_apply_function(CompilationContext &context,
        std::string const& functionName,
        ReferenceList inputs);

Term* create_comment(Branch& branch, std::string const& text);
Term* create_literal_string(CompilationContext &context, ast::LiteralString& ast);
Term* create_literal_float(CompilationContext &context, ast::LiteralFloat& ast);
Term* create_literal_integer(CompilationContext &context, ast::LiteralInteger& ast);
Term* create_dot_concatenated_call(CompilationContext &context, ast::Infix& ast);
Term* create_arrow_concatenated_call(CompilationContext &context, ast::Infix& ast);
Term* create_feedback_call(CompilationContext &context, ast::Infix& ast);
Term* create_infix_call(CompilationContext &context, ast::Infix& ast);

} // namespace circa

#endif
