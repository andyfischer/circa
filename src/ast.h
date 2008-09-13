// Copyright 2008 Andrew Fischer

#ifndef CIRCA__AST__INCLUDED
#define CIRCA__AST__INCLUDED

#include <string>
#include <sstream>
#include <vector>

#include "common_headers.h"

namespace circa {
namespace ast {
    
struct Expression;

struct ExpressionWalker
{
    virtual void visit(Expression* expr) = 0;
};

struct Expression
{
    typedef std::vector<Expression*> List;

    Expression() { }
    virtual ~Expression() { }
    virtual std::string toString() const = 0;
    virtual Term* createTerm(Branch* branch) = 0;
    virtual void walk(ExpressionWalker &walker) = 0;
    virtual std::string typeName() = 0;
};

struct Infix : public Expression
{
    std::string operatorStr;
    std::string preOperatorWhitespace;
    std::string postOperatorWhitespace;
    Expression* left;
    Expression* right;

    Infix(std::string const& _operatorStr, Expression* _left, Expression* _right)
        : operatorStr(_operatorStr), left(_left), right(_right) {}
    ~Infix();
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker);
    virtual std::string typeName() { return "infix"; }
};

struct FunctionCall : public Expression
{
    struct Argument {
        Expression* expression;
        std::string preWhitespace;
        std::string postWhitespace;

        Argument()
          : expression(NULL)
        {
        }
        ~Argument()
        {
            delete expression;
        }
    };

    typedef std::vector<Argument*> ArgumentList;

    std::string functionName;
    ArgumentList arguments;

    explicit FunctionCall(std::string const& _name) : functionName(_name) { }
    ~FunctionCall();

    void addArgument(Expression* expr, std::string const& preWhitespace,
            std::string const& postWhitespace);
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker);
    virtual std::string typeName() { return "function-call"; }
};

struct LiteralString : public Expression
{
    std::string text;

    explicit LiteralString(std::string const& _text) : text(_text) { }
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker) { walker.visit(this); }
    virtual std::string typeName() { return "literal-string"; }
};

struct LiteralFloat : public Expression
{
    std::string text;

    explicit LiteralFloat(std::string const& _text) : text(_text) { }
    virtual std::string toString() const
    {
        return text;
    }
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker) { walker.visit(this); }
    virtual std::string typeName() { return "literal-float"; }
};

struct LiteralInteger : public Expression
{
    std::string text;

    explicit LiteralInteger(std::string const& _text) : text(_text) { }
    virtual std::string toString() const
    {
        return text;
    }
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker) { walker.visit(this); }
    virtual std::string typeName() { return "literal-integer"; }
};

struct Identifier : public Expression
{
    std::string text;
    bool hasRebindOperator;

    explicit Identifier(std::string const& _text) : text(_text), hasRebindOperator(false) {}
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
    virtual void walk(ExpressionWalker &walker) { walker.visit(this); }
    virtual std::string typeName() { return "identifier"; }
};

struct Statement
{
    Expression* expression;

    Statement() : expression(NULL) {}
    virtual ~Statement() { delete expression; }
    virtual std::string toString() const = 0;
    virtual Term* createTerm(Branch* branch) = 0;
    virtual std::string typeName() { return "statement"; }

    typedef std::vector<Statement*> List;
};

struct StatementList
{
    Statement::List statements;

    void push(Statement* statement);

    ~StatementList();
    virtual std::string toString() const;
    void createTerms(Branch* branch);
    virtual std::string typeName() { return "statement-list"; }
};

struct ExpressionStatement : public Statement
{
    std::string nameBinding;
    std::string preEqualsWhitepace;
    std::string postEqualsWhitespace;

    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
    virtual std::string typeName() { return "expression-statement"; }
};

// 'IgnorableStatement' includes comments and blank lines
struct IgnorableStatement : public Statement
{
    std::string text;

    virtual ~IgnorableStatement() { }
    virtual std::string toString() const { return text; }
    virtual Term* createTerm(Branch* branch) { return NULL; }
    virtual std::string typeName() { return "ignorable-statement"; }
};

struct FunctionHeader
{
    struct Argument {
        std::string type;
        std::string name;

        Argument() {}
        Argument(std::string const& _type, std::string const& _name)
            : type(_type), name(_name) {}
    };
    typedef std::vector<Argument> ArgumentList;

    std::string functionName;
    ArgumentList arguments;
    std::string outputType;

    FunctionHeader() {}
    ~FunctionHeader() {}
    void addArgument(std::string const& type, std::string const& name);
    std::string toString() const;
};

struct FunctionDecl : public Statement
{
    FunctionHeader *header;
    StatementList *statements;

    FunctionDecl() : header(NULL), statements(NULL) {}
    ~FunctionDecl();

    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
};

} // namespace ast
} // namespace circa

#endif
