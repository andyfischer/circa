// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "function.h"
#include "parser.h"
#include "runtime.h"
#include "term.h"
#include "token_stream.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace ast {


std::string getInfixFunctionName(std::string infix)
{
    if (infix == "+")
        return "add";
    else if (infix == "-")
        return "sub";
    else if (infix == "*")
        return "mult";
    else if (infix == "/")
        return "div";
    else if (infix == "<")
        return "less-than";
    else if (infix == "<=")
        return "less-than-eq";
    else if (infix == ">")
        return "greater-than";
    else if (infix == ">=")
        return "greater-than-eq";
    else if (infix == "==")
        return "equals";
    else if (infix == "||")
        return "or";
    else if (infix == "&&")
        return "and";

    assert(false);

    return ""; // unreachable
}

Term* find_and_apply_function(CompilationContext &context, std::string const& functionName,
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

Infix::~Infix()
{
    delete left;
    delete right;
}

std::string
Infix::toString() const
{
    if (left == NULL) return "<error, left is NULL>";
    if (right == NULL) return "<error, right is NULL>";
    return this->left->toString() + this->preOperatorWhitespace
        + this->operatorStr
        + this->postOperatorWhitespace
        + this->right->toString();
}

Term*
Infix::createTerm(CompilationContext &context)
{
    // special case: right arrow or dot
    if (this->operatorStr == "->" || this->operatorStr == ".") {
        Term* leftTerm = this->left->createTerm(context);

        Identifier *rightIdent = dynamic_cast<Identifier*>(this->right);

        if (rightIdent == NULL) {
            parser::syntax_error("Right side of -> must be an identifier");
        }

        return find_and_apply_function(context, rightIdent->text, ReferenceList(leftTerm));
    }

    // another special case: :=
    if (this->operatorStr == ":=") {

        Term* leftTerm = this->left->createTerm(context);
        Term* rightTerm = this->right->createTerm(context);

        return apply_function(context.topBranch(), APPLY_FEEDBACK, ReferenceList(leftTerm, rightTerm));
    }

    std::string functionName = getInfixFunctionName(this->operatorStr);

    Term* function = context.findNamed(functionName);

    if (function == NULL) {
        parser::syntax_error(std::string("couldn't find function: ") + functionName);
        return NULL; // unreachable
    }

    Term* leftTerm = this->left->createTerm(context);
    Term* rightTerm = this->right->createTerm(context);
    
    return apply_function(context.topBranch(), function, ReferenceList(leftTerm, rightTerm));
}

void
Infix::walk(ExpressionWalker& walker)
{
    walker.visit(this);
    left->walk(walker);
    right->walk(walker);
}

FunctionCall::~FunctionCall()
{
    ArgumentList::iterator it;
    for (it = this->arguments.begin(); it != this->arguments.end(); ++it) {
        delete (*it);
    }
}

void
FunctionCall::addArgument(Expression* expr, std::string const& preWhitespace,
            std::string const& postWhitespace)
{
    Argument *arg = new Argument();
    arg->expression = expr;
    arg->preWhitespace = preWhitespace;
    arg->postWhitespace = postWhitespace;
    this->arguments.push_back(arg);
}

std::string
FunctionCall::toString() const
{
    std::stringstream output;
    output << this->functionName << "(";

    bool firstArg = true;
    ArgumentList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); ++it) {
        if (!firstArg)
            output << ",";
        output << (*it)->preWhitespace << (*it)->expression->toString()
            << (*it)->postWhitespace;
        firstArg = false;
    }
    output << ")";
    return output.str();
}

Term*
FunctionCall::createTerm(CompilationContext &context)
{
    ReferenceList inputs;

    for (unsigned int i=0; i < arguments.size(); i++) {
        Argument* arg = arguments[i];
        Term* term = arg->expression->createTerm(context);
        assert(term != NULL);
        inputs.append(term);
    }

    Term* result = find_and_apply_function(context, this->functionName, inputs);
    assert(result != NULL);
    return result;
}

void
FunctionCall::walk(ExpressionWalker& walker)
{
    walker.visit(this);
    ArgumentList::iterator it;
    for (it = arguments.begin(); it != arguments.end(); ++it) {
        if (*it == NULL)
            throw std::runtime_error("argument is null");
        Expression* expr = (*it)->expression;
        if (expr == NULL)
            throw std::runtime_error("expression is null");
        expr->walk(walker);
    }
}

std::string
LiteralString::toString() const
{
    return "\"" + this->text + "\"";
}

Term*
LiteralString::createTerm(CompilationContext &context)
{
    return string_var(context.topBranch(), this->text);
}

Term*
LiteralFloat::createTerm(CompilationContext &context)
{
    float value = atof(this->text.c_str());
    Term* term = float_var(context.topBranch(), value);
    float mutability = hasQuestionMark ? 1.0 : 0.0;
    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    return term;
}

Term*
LiteralInteger::createTerm(CompilationContext &context)
{
    int value = atoi(this->text.c_str());
    return int_var(context.topBranch(), value);
}

std::string
Identifier::toString() const
{
    std::string rebindSymbol = "";
    if (hasRebindOperator)
        rebindSymbol = "@";
    return rebindSymbol + text;
}

Term*
Identifier::createTerm(CompilationContext &context)
{
    Term* result = context.findNamed(this->text);

    if (result == NULL) {
        parser::syntax_error(std::string("Couldn't find identifier: ") + this->text);
    }

    assert(result != NULL);
    return result;
}

std::string
ExpressionStatement::toString() const
{
    std::string output;

    if (nameBinding != "") {
        output = nameBinding + preEqualsWhitepace + "=" + postEqualsWhitespace;
    }

    output += expression->toString();
    
    return output;
}

Term*
ExpressionStatement::createTerm(CompilationContext &context)
{
    Term* term = this->expression->createTerm(context);
    
    if (this->nameBinding != "")
        context.topBranch().bindName(term, this->nameBinding);

    return term;
}

StatementList::~StatementList()
{
    Statement::List::iterator it;
    for (it = statements.begin(); it != statements.end(); ++it) {
        delete (*it);
    }
}

void
StatementList::push(Statement* statement)
{
    this->statements.push_back(statement);
}

std::string
StatementList::toString() const
{
    std::stringstream output;

    Statement::List::const_iterator it;
    for (it = statements.begin(); it != statements.end(); ++it) {
        output << (*it)->toString() << "\n";
    }
    return output.str();
}

void
StatementList::createTerms(CompilationContext &context)
{
    Statement::List::const_iterator it;
    for (it = statements.begin(); it != statements.end(); ++it) {
        (*it)->createTerm(context);
    }
}

void
FunctionHeader::addArgument(std::string const& type, std::string const& name)
{
    Argument arg;
    arg.type = type;
    arg.name = name;
    this->arguments.push_back(arg);
}

std::string
FunctionHeader::toString() const
{
    std::stringstream out;
    out << "function " << functionName << "(";
    ArgumentList::const_iterator it;
    bool first = true;
    for (it = arguments.begin(); it != arguments.end(); ++it) {
        if (!first) out << ", ";
        out << it->type << " " << it->name;
        first = false;
    }
    out << ")";
    return out.str();
}

FunctionDecl::~FunctionDecl()
{
    delete this->header;
    delete this->statements;
}

Term*
FunctionDecl::createTerm(CompilationContext &context)
{
    // Make a workspace where we'll assemble this function
    Branch workspace;

    Term* inputTypes = eval_statement(workspace, "inputTypes = list()");

    for (unsigned int inputIndex=0;
         inputIndex < this->header->arguments.size();
         inputIndex++)
    {
        FunctionHeader::Argument &arg = this->header->arguments[inputIndex];
        Term* term = context.topBranch().findNamed(arg.type);
        if (term == NULL)
            parser::syntax_error(std::string("Identifier not found (input type): ") + arg.type);

        if (!is_type(term))
            parser::syntax_error(std::string("Identifier is not a type: ") + arg.type);

        set_input(inputTypes, inputIndex, term);
    }

    Term* outputType = NULL;

    if (this->header->outputType == "") {
        outputType = VOID_TYPE;
    } else {
        outputType = context.findNamed(this->header->outputType);
        if (outputType == NULL)
            parser::syntax_error(std::string("Identifier not found (output type): ") + this->header->outputType);
        if (!is_type(outputType))
            parser::syntax_error(std::string("Identifier is not a type: ") + this->header->outputType);
    }

    // Load into workspace
    string_var(workspace, this->header->functionName, "functionName");
    workspace.bindName(outputType, "outputType");

    // Create
    Term* sub = eval_statement(workspace,
        "sub = subroutine-create(functionName, inputTypes, outputType)");

    // Name each input
    for (unsigned int inputIndex=0;
         inputIndex < this->header->arguments.size();
         inputIndex++)
    {
        std::string name = this->header->arguments[inputIndex].name;
        int_var(workspace, inputIndex, "inputIndex");
        string_var(workspace, name, "name");

        sub = eval_statement(workspace,
                "function-name-input(@sub, inputIndex, name)");
    }

    // Apply every statement
    context.push(&as_function(sub).subroutineBranch, NULL);
    int numStatements = this->statements->count();
    for (int statementIndex=0; statementIndex < numStatements; statementIndex++) {
        Statement* statement = this->statements->operator[](statementIndex);

        statement->createTerm(context);
    }
    context.pop();

    Term* outer_val = create_var(&context.topBranch(), FUNCTION_TYPE);
    steal_value(sub, outer_val);
    context.topBranch().bindName(outer_val, this->header->functionName);

    return sub;
}

std::string
FunctionDecl::toString() const
{
    std::stringstream out;

    out << header->toString() << std::endl;

    out << statements->toString();

    out << "end";

    return out.str();
}

std::string
TypeDecl::toString() const
{
    std::stringstream out;

    out << "type " << name << "{" << std::endl;

    MemberList::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        out << "  " << it->type << " " << it->name << std::endl;
    }

    out << "}";

    return out.str();
}

Term*
TypeDecl::createTerm(CompilationContext &context)
{
    Branch workspace;

    string_var(workspace, this->name, "typeName");

    eval_statement(workspace, "t = create-compound-type(typeName)");

    MemberList::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        //string_var(workspace, it->type, "fieldType");
        string_var(workspace, it->name, "fieldName");
        eval_statement(workspace,
            std::string("compound-type-append-field(@t, "+it->type+", fieldName)"));
    }

    Term* result_term = create_var(&context.topBranch(), TYPE_TYPE);
    steal_value(workspace["t"], result_term);
    context.topBranch().bindName(result_term, this->name);
    return result_term;
}

std::string
IfStatement::toString() const
{
    return "IfStatment::toString TODO";
}

Term*
IfStatement::createTerm(CompilationContext &context)
{
    assert(this->condition != NULL);
    assert(this->positiveBranch != NULL);

    Term* conditionTerm = this->condition->createTerm(context);

    Term* ifStatementTerm = apply_function(context.topBranch(), "if-statement", ReferenceList(conditionTerm));

    Branch& posBranch = as_branch(ifStatementTerm->state->field(0));
    Branch& negBranch = as_branch(ifStatementTerm->state->field(1));

    assert(posBranch.owningTerm == ifStatementTerm);

    context.push(&posBranch, ifStatementTerm->state->field(0));
    this->positiveBranch->createTerms(context);
    context.pop();

    if (this->negativeBranch != NULL) {
        context.push(&negBranch, ifStatementTerm->state->field(1));
        this->negativeBranch->createTerms(context);
        context.pop();
    }

    return ifStatementTerm;
}

} // namespace ast
} // namespace circa
