// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

namespace circa {

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
};

Term* find_and_apply_function(CompilationContext &context,
        std::string const& functionName,
        ReferenceList inputs);

Term* create_comment(Branch& branch, std::string const& text);

} // namespace circa

#endif
