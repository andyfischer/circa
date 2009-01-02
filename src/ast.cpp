// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "cpp_interface.h"
#include "function.h"
#include "parser.h"
#include "list.h"
#include "runtime.h"
#include "set.h"
#include "term.h"
#include "token_stream.h"
#include "type.h"
#include "wrappers.h"
#include "values.h"

namespace circa {
namespace ast {

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
    // special case for dot operator. Try to find a member function.
    // If not found, apply right term as a function
    if (this->operatorStr == ".") {
        Term* leftTerm = this->left->createTerm(context);

        // Figure out the function name. Right expression might be
        // an identifier or a function call
        std::string functionName;

        if (right->typeName() == "Identifier")
            functionName = dynamic_cast<Identifier*>(right)->text;
        else if (right->typeName() == "FunctionCall")
            functionName = dynamic_cast<FunctionCall*>(right)->functionName;
        else
            parser::syntax_error(right->typeName() + " on right side of infix dot");

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

        if (right->typeName() == "FunctionCall") {
            FunctionCall* functionCall = dynamic_cast<FunctionCall*>(right);

            for (unsigned int i=0; i < functionCall->arguments.size(); i++) {
                FunctionCall::Argument *arg = functionCall->arguments[i];
                Term *term = arg->expression->createTerm(context);
                inputs.append(term);
            }
        }

        Term* result = apply_function(context.topBranch(), function, inputs);

        if (memberFunctionCall
                && (left->typeName() == "Identifier")
                && (right->typeName() == "FunctionCall")) {

            // Rebind this identifier
            std::string id = dynamic_cast<Identifier*>(left)->text;

            context.topBranch().bindName(result, id);
        }

        return result;
    }

    // special case for right arrow. Apply the right term as a function
    if (this->operatorStr == "->") {
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
    return string_value(context.topBranch(), this->text);
}

Term*
LiteralFloat::createTerm(CompilationContext &context)
{
    float value = atof(this->text.c_str());
    Term* term = float_value(context.topBranch(), value);
    float mutability = hasQuestionMark ? 1.0 : 0.0;
    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    return term;
}

Term*
LiteralInteger::createTerm(CompilationContext &context)
{
    int value = atoi(this->text.c_str());
    return int_value(context.topBranch(), value);
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
    // Make sure our expression is not just an identifier.
    if (expression->typeName() == "Identifier" && nameBinding != "") {
        parser::syntax_error("Renaming an identifier is not currently supported");
    }

    Term* term = this->expression->createTerm(context);
    
    if (this->nameBinding != "")
        context.topBranch().bindName(term, this->nameBinding);

    return term;
}

StatementList::~StatementList()
{
    Statement::Vector::iterator it;
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

    Statement::Vector::const_iterator it;
    for (it = statements.begin(); it != statements.end(); ++it) {
        output << (*it)->toString() << "\n";
    }
    return output.str();
}

void
StatementList::createTerms(CompilationContext &context)
{
    Statement::Vector::const_iterator it;
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

    ReferenceList inputTypes;

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

        inputTypes.append(term);
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
    Term* inputTypesTerm = workspace.eval("inputTypes = tuple()");
    as<ReferenceList>(inputTypesTerm) = inputTypes;
    string_value(workspace, this->header->functionName, "functionName");
    workspace.bindName(outputType, "outputType");

    // Create
    Term* sub = workspace.eval(
        "sub = subroutine-create(functionName, inputTypes, outputType)");

    // Name each input
    for (unsigned int inputIndex=0;
         inputIndex < this->header->arguments.size();
         inputIndex++)
    {
        std::string name = this->header->arguments[inputIndex].name;
        int_value(workspace, inputIndex, "inputIndex");
        string_value(workspace, name, "name");

        sub = workspace.eval(
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

    Term* outer_val = create_value(&context.topBranch(), FUNCTION_TYPE);
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

    string_value(workspace, this->name, "typeName");

    workspace.eval("t = create-compound-type(typeName)");

    MemberList::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        //string_value(workspace, it->type, "fieldType");
        string_value(workspace, it->name, "fieldName");
        workspace.eval(
            std::string("compound-type-append-field(@t, "+it->type+", fieldName)"));
    }

    Term* result_term = create_value(&context.topBranch(), TYPE_TYPE);
    steal_value(workspace["t"], result_term);
    context.topBranch().bindName(result_term, this->name);
    return result_term;
}

IfStatement::~IfStatement()
{
    delete condition;
    delete positiveBranch;
    delete negativeBranch;
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
    Branch& joiningTermsBranch = as_branch(ifStatementTerm->state->field(2));

    context.push(&posBranch, ifStatementTerm->state->field(0));
    this->positiveBranch->createTerms(context);
    context.pop();

    if (this->negativeBranch != NULL) {
        context.push(&negBranch, ifStatementTerm->state->field(1));
        this->negativeBranch->createTerms(context);
        context.pop();
    }

    // Create joining terms

    // First, get a list of all names that were bound in these branches

    Branch workspace;

    create_value(&workspace, "Branch", &posBranch, "posBranch");
    create_value(&workspace, "Branch", &negBranch, "negBranch");

    workspace.eval("names = posBranch.get-branch-bound-names");
    workspace.eval("list-join(@names, negBranch.get-branch-bound-names)");

    workspace.eval("list-remove-duplicates(@names)");

    List& names = as<List>(workspace["names"]);
    Branch& outerBranch = context.topBranch();

    // Remove any names which are not bound in the outer branch
    for (int i=0; i < names.count(); i++) {
        if (!outerBranch.containsName(as_string(names[i]))) {
            names.remove(i--);
        }
    }

    // For each name, create joining term
    for (int i=0; i < names.count(); i++) {
        std::string &name = as_string(names[i]);

        Term* posTerm = posBranch.containsName(name) ? posBranch[name] : outerBranch.findNamed(name);
        Term* negTerm = negBranch.containsName(name) ? negBranch[name] : outerBranch.findNamed(name);

        Term* joining_term = apply_function(joiningTermsBranch,
                "if-expr",
                ReferenceList(conditionTerm, posTerm, negTerm));
        outerBranch.bindName(joining_term, name);
    }

    return ifStatementTerm;
}

void
output_ast_to_string(ASTNode *node, std::stringstream &out)
{
    int numChildren = node->numChildren();

    if (numChildren > 0) {
        out << "(" << node->typeName();
        for (int i=0; i < numChildren; i++)
        {
            out << " ";
            output_ast_to_string(node->getChild(i), out);
        }
        out << ")";
    } else {
        out << node->typeName();
    }
}

std::string
print_ast(ASTNode *node)
{
    std::stringstream stream;
    output_ast_to_string(node, stream);
    return stream.str();
}

} // namespace ast
} // namespace circa
