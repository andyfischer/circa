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
    struct Argument {
        Expression* expression;
        std::string preWhitespace;
        std::string postWhitespace;

        Argument::Argument()
          : expression(NULL)
        {
        }
        Argument::~Argument()
        {
            delete expression;
        }
    };

    typedef std::vector<Argument*> ArgumentList;

    std::string functionName;
    ArgumentList arguments;

    explicit FunctionCall(std::string const& _name)
      : functionName(_name)
    {
    }

    void addArgument(Expression* expr, std::string const& preWhitespace,
            std::string const& postWhitespace)
    {
        Argument *arg = new Argument();
        arg->expression = expr;
        arg->preWhitespace = preWhitespace;
        arg->postWhitespace = postWhitespace;
        this->arguments.push_back(arg);
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

struct LiteralInteger : public Expression
{
    std::string text;

    explicit LiteralInteger(std::string const& _text)
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
