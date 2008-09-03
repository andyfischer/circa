#ifndef CIRCA__AST__INCLUDED
#define CIRCA__AST__INCLUDED

#include <string>
#include <sstream>
#include <vector>

#include "common_headers.h"

namespace circa {
namespace ast {

struct Expression
{
    typedef std::vector<Expression*> List;

    Expression() { }
    virtual ~Expression() { }
    virtual std::string toString() const = 0;
    virtual Term* createTerm(Branch* branch) = 0;
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

    void addArgument(Expression* expr, std::string const& preWhitespace,
            std::string const& postWhitespace);
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
};

struct LiteralString : public Expression
{
    std::string text;

    explicit LiteralString(std::string const& _text) : text(_text) { }
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
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
};

struct Identifier : public Expression
{
    std::string text;

    explicit Identifier(std::string const& _text) : text(_text) { }
    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
};

struct Statement {
    typedef std::vector<Statement*> List;

    virtual std::string toString() const = 0;
    virtual Term* createTerm(Branch* branch) = 0;
};

struct StatementList
{
    Statement::List statements;

    void push(Statement* statement);

    virtual std::string toString() const;
    void createTerms(Branch* branch);
};

struct ExpressionStatement : public Statement
{
    std::string nameBinding;
    Expression* expression;
    std::string preEqualsWhitepace;
    std::string postEqualsWhitespace;

    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
};

// 'IgnorableStatement' includes comments and blank lines
struct IgnorableStatement : public Statement
{
    std::string text;

    virtual std::string toString() const { return text; }
    virtual Term* createTerm(Branch* branch) { return NULL; }
};

struct FunctionDecl : public Statement
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
    StatementList *statements;

    FunctionDecl() : statements(NULL) {}
    FunctionDecl(std::string const& _functionName)
        : functionName(_functionName), statements(NULL) {}
    ~FunctionDecl();

    void addArgument(std::string const& type, std::string const& name);

    virtual std::string toString() const;
    virtual Term* createTerm(Branch* branch);
};

} // namespace ast
} // namespace circa

#endif
