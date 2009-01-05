// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "ref_list.h"
#include "runtime.h"
#include "term.h"
#include "values.h"

namespace circa {

Term*
CompilationContext::findNamed(std::string const& name) const
{
    assert(!scopeStack.empty());

    for (size_t i = scopeStack.size() - 1; i >= 0; i--) {

        Term* term = scopeStack[i].branch->findNamed(name);

        if (term != NULL)
            return term;
    }

    return NULL;
}

CompilationContext::Scope const&
CompilationContext::topScope() const
{
    return scopeStack.back();
}

Branch&
CompilationContext::topBranch() const
{
    assert(!scopeStack.empty());
    return *scopeStack.back().branch;
}

void
CompilationContext::pushScope(Branch* branch, Term* branchOwner)
{
    scopeStack.push_back(Scope(branch, branchOwner));
}

void
CompilationContext::pushExpressionFrame(bool insideExpression)
{
    expressionStack.push_back(ExpressionFrame(insideExpression));
}

void
CompilationContext::popExpressionFrame()
{
    expressionStack.pop_back();
}

bool
CompilationContext::isInsideExpression() const
{
    if (expressionStack.empty())
        return false;

    return expressionStack.back().insideExpression;
}

void
CompilationContext::popScope()
{
    scopeStack.pop_back();
}

Term* find_and_apply_function(CompilationContext &context,
        std::string const& functionName,
        ReferenceList inputs)
{
    Term* function = context.findNamed(functionName);

    if (function == NULL) {
        std::cout << "warning: function not found: " << functionName << std::endl;

        Term* result = apply_function(context.topBranch(), UNKNOWN_FUNCTION, inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply_function(context.topBranch(), function, inputs);
}

Term* create_comment(Branch& branch, std::string const& text)
{
    Term* result = apply_function(branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = text;
    return result;
}

Term* create_literal_string(CompilationContext &context, ast::LiteralString ast)
{
    Term* term = string_value(context.topBranch(), ast.text);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = context.isInsideExpression();
    return term;
}

Term* create_literal_float(CompilationContext &context, ast::LiteralFloat& ast)
{
    float value = atof(ast.text.c_str());
    Term* term = float_value(context.topBranch(), value);
    float mutability = ast.hasQuestionMark ? 1.0 : 0.0;
    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = context.isInsideExpression();
    return term;
}

Term* create_literal_integer(CompilationContext &context, ast::LiteralInteger& ast)
{
    int value = atoi(ast.text.c_str());
    Term* term = int_value(context.topBranch(), value);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = context.isInsideExpression();
    return term;
}

} // namespace circa
