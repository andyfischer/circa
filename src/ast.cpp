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

Term*
Infix::createTerm(CompilationContext &context)
{
    // special case for dot operator.
    if (this->operatorStr == ".") {
        return create_dot_concatenated_call(context, *this);
    }

    // special case for ->
    if (this->operatorStr == "->") {
        return create_arrow_concatenated_call(context, *this);
    }

    // special case for :=
    if (this->operatorStr == ":=") {
        return create_feedback_call(context, *this);
    }

    return create_infix_call(context, *this);
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

Term*
FunctionCall::createTerm(CompilationContext &context)
{
    return create_function_call(context, *this);
}

Term*
LiteralString::createTerm(CompilationContext &context)
{
    return create_literal_string(context, *this);
}

Term*
LiteralFloat::createTerm(CompilationContext &context)
{
    return create_literal_float(context, *this);
}

Term*
LiteralInteger::createTerm(CompilationContext &context)
{
    return create_literal_integer(context, *this);
}

Term*
Identifier::createTerm(CompilationContext &context)
{
    Term* result = context.findNamed(this->text);

    if (result == NULL) {
        // parser::syntax_error(std::string("Couldn't find identifier: ") + this->text);
        return NULL;
    }

    if (hasRebindOperator) {
        assert(context.pendingRebind == "");
        context.pendingRebind = this->text;
    }

    return result;
}

Term*
ExpressionStatement::createTerm(CompilationContext &context)
{
    // Make sure our expression is not just an identifier.
    if (expression->typeName() == "Identifier" && nameBinding != "") {
        parser::syntax_error("Renaming an identifier is not currently supported");
    }

    context.pushExpressionFrame(false);

    Term* term = this->expression->createTerm(context);

    context.popExpressionFrame();
    
    if (this->nameBinding != "")
        context.topBranch().bindName(term, this->nameBinding);

    if (context.pendingRebind != "") {
        context.topBranch().bindName(term, context.pendingRebind);
        context.pendingRebind = "";
    }

    term->syntaxHints.precedingWhitespace = this->precedingWhitespace;

    return term;
}

Term*
CommentStatement::createTerm(CompilationContext &context)
{
    return create_comment(context.topBranch(), this->text);
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

FunctionDecl::~FunctionDecl()
{
    delete this->header;
    delete this->statements;
}

Term*
FunctionDecl::createTerm(CompilationContext &context)
{
    Term* resultTerm = create_value(&context.topBranch(), FUNCTION_TYPE, this->header->functionName);
    Function& result = as_function(resultTerm);
    result.name = header->functionName;
    result.evaluate = Function::subroutine_call_evaluate;
    result.stateType = BRANCH_TYPE;

    for (unsigned int inputIndex=0;
         inputIndex < this->header->arguments.size();
         inputIndex++)
    {
        FunctionHeader::Argument &arg = this->header->arguments[inputIndex];
        Term* term = find_named(&context.topBranch(), arg.type);
        if (term == NULL)
            parser::syntax_error(std::string("Identifier not found (input type): ") + arg.type);

        if (!is_type(term))
            parser::syntax_error(std::string("Identifier is not a type: ") + arg.type);

        result.inputTypes.append(term);

        Function::InputProperties inputProps;
        inputProps.name = arg.name;
        result.inputProperties.push_back(inputProps);
    }

    if (this->header->outputType == "") {
        result.outputType = VOID_TYPE;
    } else {
        result.outputType = context.findNamed(this->header->outputType);
        if (result.outputType == NULL)
            parser::syntax_error(std::string("Identifier not found (output type): ") + this->header->outputType);
        if (!is_type(result.outputType))
            parser::syntax_error(std::string("Identifier is not a type: ") + this->header->outputType);
    }

    // Syntax hints
    resultTerm->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;

    // Create input placeholders
    for (unsigned int inputIndex=0;
         inputIndex < this->header->arguments.size();
         inputIndex++)
    {
        std::string name = this->header->arguments[inputIndex].name;

        if (name == "")
            name = get_placeholder_name_for_index(inputIndex);

        /*Term* placeholder =*/ create_value(&result.subroutineBranch, result.inputTypes[inputIndex], name);

    }

    result.subroutineBranch.outerScope = &context.topBranch();

    // Apply every statement
    context.pushScope(&result.subroutineBranch, NULL);
    int numStatements = this->statements->count();
    for (int statementIndex=0; statementIndex < numStatements; statementIndex++) {
        Statement* statement = this->statements->operator[](statementIndex);

        statement->createTerm(context);
    }
    context.popScope();

    return resultTerm;
}

Term*
TypeDecl::createTerm(CompilationContext &context)
{
    Term* result_term = create_value(&context.topBranch(), TYPE_TYPE);
    Type &type = as_type(result_term);
    type.makeCompoundType(this->name);

    MemberList::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        type.addField(context.findNamed(it->type), it->name);
        type.syntaxHints.addField(it->name);
    }

    context.topBranch().bindName(result_term, this->name);
    return result_term;
}

IfStatement::~IfStatement()
{
    delete condition;
    delete positiveBranch;
    delete negativeBranch;
}

Term*
IfStatement::createTerm(CompilationContext &context)
{
    assert(this->condition != NULL);
    assert(this->positiveBranch != NULL);

    context.pushExpressionFrame(true);
    Term* conditionTerm = this->condition->createTerm(context);
    context.popExpressionFrame();

    Term* ifStatementTerm = apply_function(&context.topBranch(),
                                           "if-statement",
                                           ReferenceList(conditionTerm));

    Branch& posBranch = as_branch(ifStatementTerm->state->field(0));
    Branch& negBranch = as_branch(ifStatementTerm->state->field(1));
    Branch& joiningTermsBranch = as_branch(ifStatementTerm->state->field(2));

    posBranch.outerScope = &context.topBranch();
    negBranch.outerScope = &context.topBranch();
    joiningTermsBranch.outerScope = &context.topBranch();

    context.pushScope(&posBranch, ifStatementTerm->state->field(0));
    this->positiveBranch->createTerms(context);
    context.popScope();

    if (this->negativeBranch != NULL) {
        context.pushScope(&negBranch, ifStatementTerm->state->field(1));
        this->negativeBranch->createTerms(context);
        context.popScope();
    }

    // Create joining terms

    // First, get a list of all names that were bound in these branches

    Branch workspace;

    import_value(workspace, BRANCH_TYPE, &posBranch, "posBranch");
    import_value(workspace, BRANCH_TYPE, &negBranch, "negBranch");

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

        Term* posTerm = posBranch.containsName(name) ?
            posBranch[name] : find_named(&outerBranch, name);
        Term* negTerm = negBranch.containsName(name) ?
            negBranch[name] : find_named(&outerBranch, name);

        Term* joining_term = apply_function(&joiningTermsBranch,
                "if-expr",
                ReferenceList(conditionTerm, posTerm, negTerm));
        outerBranch.bindName(joining_term, name);
    }

    return ifStatementTerm;
}

Term* StatefulValueDeclaration::createTerm(CompilationContext &context)
{
    return create_stateful_value_declaration(context, *this);
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
