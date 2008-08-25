#ifndef CIRCA__AST__INCLUDED
#define CIRCA__AST__INCLUDED

#include <string>
#include <sstream>
#include <vector>

namespace circa {
namespace ast {

struct Expression
{
    typedef std::vector<Expression*> List;

    Expression() { }
    virtual ~Expression() { }
    virtual std::string toString() const = 0;
};

struct Infix : public Expression
{
    std::string operatorStr;
    std::string preOperatorWhitespace;
    std::string postOperatorWhitespace;
    Expression* left;
    Expression* right;

    Infix();
    ~Infix();
    virtual std::string toString() const { return "todo"; }
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

    void addArgument(std::string const& preWhitespace, Expression* expr,
            std::string const& postWhitespace);
    virtual std::string toString() const;
};

struct LiteralString : public Expression
{
    std::string text;

    explicit LiteralString(std::string const& _text)
      : text(_text)
    {
    }
    virtual std::string toString() const
    {
        return text;
    }
};

struct LiteralFloat : public Expression
{
    std::string text;

    explicit LiteralFloat(std::string const& _text)
      : text(_text)
    {
    }
    virtual std::string toString() const
    {
        return text;
    }
};

struct LiteralInteger : public Expression
{
    std::string text;

    explicit LiteralInteger(std::string const& _text)
      : text(_text)
    {
    }
    virtual std::string toString() const
    {
        return text;
    }
};

struct Identifier : public Expression
{
    std::string text;

    explicit Identifier(std::string const& _text)
      : text(_text)
    {
    }
    virtual std::string toString() const
    {
        return text;
    }
};

struct Statement
{
    typedef std::vector<Statement*> List;

    std::string nameBinding;
    Expression* expression;
    std::string preEqualsWhitepace;
    std::string postEqualsWhitespace;

    virtual std::string toString() const
    {
        std::string output;

        if (nameBinding != "") {
            output = nameBinding + preEqualsWhitepace + "=" + postEqualsWhitespace;
        }

        output += expression->toString();
        
        return output;
    }
};

struct StatementList
{
    Statement::List statements;

    void push(Statement* statement);

    virtual std::string toString() const
    {
        std::stringstream output;

        Statement::List::const_iterator it;
        for (it = statements.begin(); it != statements.end(); ++it) {
            output << (*it)->toString() << "\n";
        }
        return output.str();
    }
};

} // namespace ast
} // namespace circa

#endif
