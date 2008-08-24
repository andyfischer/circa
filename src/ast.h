#ifndef CIRCA__AST__INCLUDED
#define CIRCA__AST__INCLUDED

#include <string>
#include <vector>

namespace circa {
namespace ast {

class Expression
{
public:
    typedef std::vector<Expression*> List;

    Expression()
    {
    }
    virtual ~Expression()
    {
    }
};

class Infix : public Expression
{
public:
    std::string mOperator;
    Expression* mLeft;
    Expression* mRight;

    Infix()
      : mLeft(NULL), mRight(NULL)
    {
    }

    ~Infix()
    {
        delete mLeft;
        delete mRight;
    }
};

class FunctionCall : public Expression
{
public:
    std::string mFunctionName;
    Expression::List mInputs;
    std::string mPreInputWhitespace;
    std::string mPostInputWhitespace;

    explicit FunctionCall(std::string const& name)
      : mFunctionName(name)
    {
    }
};

class LiteralString : public Expression
{
public:
    std::string mText;

    explicit LiteralString(std::string const& text)
      : mText(text)
    {
    }
};

class LiteralFloat : public Expression
{
    std::string mText;

public:
    explicit LiteralFloat(std::string const& text)
      : mText(text)
    {
    }
};

class Identifier : public Expression
{
    std::string mText;

public:
    explicit Identifier(std::string const& text)
      : mText(text)
    {
    }
};
    

class Statement
{
public:
    std::string mNameBinding;
    Expression* mExpression;
    std::string mPreEqualsWhitepace;
    std::string mPostEqualsWhitespace;

    typedef std::vector<Statement*> List;
};


class StatementList
{
public:
    Statement::List mStatements;

    void push(Statement* statement)
    {
        mStatements.push_back(statement);
    }
};


} // namespace ast
} // namespace circa

#endif
