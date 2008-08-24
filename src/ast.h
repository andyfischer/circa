#ifndef CIRCA__AST__INCLUDED
#define CIRCA__AST__INCLUDED

#include <string>
#include <vector>

namespace circa {
namespace ast {

struct Expression
{
    typedef std::vector<Expression*> List;

    Expression()
    {
    }
    virtual ~Expression()
    {
    }
};

struct Infix : public Expression
{
    std::string operatorStr;
    Expression* left;
    Expression* right;

    Infix()
      : left(NULL), right(NULL)
    {
    }

    ~Infix()
    {
        delete left;
        delete right;
    }
};

struct FunctionCall : public Expression
{
    std::string functionName;
    Expression::List inputs;
    std::string preInputWhitespace;
    std::string postInputWhitespace;

    explicit FunctionCall(std::string const& _name)
      : functionName(_name)
    {
    }

    void addInput(Expression* input)
    {
        this->inputs.push_back(input);
    }
};

struct LiteralString : public Expression
{
    std::string text;

    explicit LiteralString(std::string const& _text)
      : text(_text)
    {
    }
};

struct LiteralFloat : public Expression
{
    std::string text;

    explicit LiteralFloat(std::string const& _text)
      : text(_text)
    {
    }
};

struct Identifier : public Expression
{
    std::string text;

    explicit Identifier(std::string const& _text)
      : text(_text)
    {
    }
};
    

struct Statement
{
    std::string nameBinding;
    Expression* expression;
    std::string preEqualsWhitepace;
    std::string postEqualsWhitespace;

    typedef std::vector<Statement*> List;
};


struct StatementList
{
    Statement::List statements;

    void push(Statement* statement)
    {
        this->statements.push_back(statement);
    }
};


} // namespace ast
} // namespace circa

#endif
