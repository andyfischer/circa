// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "parser.h"
#include "ref_list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

Term*
CompilationContext::findNamed(std::string const& name) const
{
    assert(!scopeStack.empty());

    for (int i = (int) (scopeStack.size() - 1); i >= 0; i--) {

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

    // If function is not found, produce an instance of unknown-function
    if (function == NULL) {
        Term* result = apply_function(context.topBranch(),
                                      UNKNOWN_FUNCTION,
                                      inputs);
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

Term* create_literal_string(CompilationContext &context, ast::LiteralString& ast)
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

Term* create_dot_concatenated_call(CompilationContext &context,
                                   ast::Infix& ast)
{
    context.pushExpressionFrame(true);
    Term* leftTerm = ast.left->createTerm(context);
    context.popExpressionFrame();

    // Figure out the function name. Right expression might be
    // an identifier or a function call
    std::string functionName;

    if (ast.right->typeName() == "Identifier")
        functionName = dynamic_cast<ast::Identifier*>(ast.right)->text;
    else if (ast.right->typeName() == "FunctionCall")
        functionName = dynamic_cast<ast::FunctionCall*>(ast.right)->functionName;
    else
        parser::syntax_error(ast.right->typeName() + " on right side of infix dot");

    Type &leftType = as_type(leftTerm->type);

    // Try to find the function. Check the type's member function first.
    Term* function = NULL;

    bool memberFunctionCall = false;

    if (leftType.memberFunctions.contains(functionName)) {
        function = leftType.memberFunctions[functionName];
        memberFunctionCall = true;

    } else {
        function = context.findNamed(functionName);
    }

    if (function == NULL)
        parser::syntax_error(functionName + " function not found.");

    // Assemble input list
    ReferenceList inputs(leftTerm);

    if (ast.right->typeName() == "FunctionCall") {
        ast::FunctionCall* functionCall = dynamic_cast<ast::FunctionCall*>(ast.right);

        for (unsigned int i=0; i < functionCall->arguments.size(); i++) {
            ast::FunctionCall::Argument *arg = functionCall->arguments[i];
            context.pushExpressionFrame(true);
            Term *term = arg->expression->createTerm(context);
            context.popExpressionFrame();
            inputs.append(term);
        }
    }

    Term* result = apply_function(context.topBranch(), function, inputs);

    if (memberFunctionCall
            && (ast.left->typeName() == "Identifier")
            && (ast.right->typeName() == "FunctionCall")) {

        // Rebind this identifier
        std::string id = dynamic_cast<ast::Identifier*>(ast.left)->text;

        context.topBranch().bindName(result, id);
    }

    TermSyntaxHints::InputSyntax leftInputSyntax;
    if (ast.left->typeName() == "Identifier")
        leftInputSyntax.style = TermSyntaxHints::InputSyntax::BY_NAME;
    else 
        leftInputSyntax.style = TermSyntaxHints::InputSyntax::BY_SOURCE;

    result->syntaxHints.inputSyntax.push_back(leftInputSyntax);
    result->syntaxHints.declarationStyle = TermSyntaxHints::DOT_CONCATENATION;
    result->syntaxHints.occursInsideAnExpression = context.isInsideExpression();

    return result;
}

Term* create_arrow_concatenated_call(CompilationContext &context, ast::Infix& ast)
{
    context.pushExpressionFrame(true);
    Term* leftTerm = ast.left->createTerm(context);
    context.popExpressionFrame();

    ast::Identifier *rightIdent = dynamic_cast<ast::Identifier*>(ast.right);

    if (rightIdent == NULL) {
        parser::syntax_error("Right side of -> must be an identifier");
    }

    return find_and_apply_function(context, rightIdent->text, ReferenceList(leftTerm));
}

Term* create_feedback_call(CompilationContext &context, ast::Infix& ast)
{
    context.pushExpressionFrame(true);
    Term* leftTerm = ast.left->createTerm(context);
    Term* rightTerm = ast.right->createTerm(context);
    context.popExpressionFrame();

    return apply_function(context.topBranch(), APPLY_FEEDBACK, ReferenceList(leftTerm, rightTerm));
}

} // namespace circa
