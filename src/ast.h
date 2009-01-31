// Copyright 2008 Andrew Fischer

#ifndef CIRCA_AST_INCLUDED
#define CIRCA_AST_INCLUDED

#include <string>
#include <sstream>
#include <vector>

#include "common_headers.h"

#include "compilation.h"

namespace circa {
namespace ast {

struct Expression;

struct ASTNode
{
    virtual ~ASTNode() {}
    virtual std::string typeName() const = 0;
    virtual int numChildren() const = 0;
    virtual ASTNode* getChild(int index) const = 0;
};

struct Expression : public ASTNode
{
    typedef std::vector<Expression*> Vector;

    Expression() { }
    virtual ~Expression() { }
    virtual Term* createTerm(CompilationContext &context) = 0;
    
    // for ASTNode
    virtual std::string typeName() const = 0;
    virtual int numChildren() const = 0;
    virtual ASTNode* getChild(int index) const = 0;
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
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "Infix"; }

    virtual int numChildren() const { return 2; }
    virtual ASTNode* getChild(int index) const
    {
        if (index == 0) return left;
        if (index == 1) return right;
        return NULL;
    }
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

    explicit FunctionCall(std::string const& _name) : functionName(_name) {}
    ~FunctionCall();

    void addArgument(Expression* expr, std::string const& preWhitespace,
            std::string const& postWhitespace);
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "FunctionCall"; }
    virtual int numChildren() const { return arguments.size(); }
    virtual ASTNode* getChild(int index) const
    {
        return arguments[index]->expression;
    }
};

struct Literal : public Expression
{
    bool hasQuestionMark;

    Literal() : hasQuestionMark(false) {}
};

struct LiteralString : public Literal
{
    std::string text;

    explicit LiteralString(std::string const& _text) : text(_text) { }
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "LiteralString"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct LiteralFloat : public Literal
{
    std::string text;

    explicit LiteralFloat(std::string const& _text) : text(_text) { }
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "LiteralFloat"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct LiteralInteger : public Literal
{
    std::string text;

    explicit LiteralInteger(std::string const& _text) : text(_text) { }
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "LiteralInteger"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct Identifier : public Expression
{
    std::string text;
    bool hasRebindOperator;

    explicit Identifier(std::string const& _text) : text(_text), hasRebindOperator(false) {}
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "Identifier"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct Statement : public ASTNode
{
    std::string precedingWhitespace;
    Expression* expression;

    Statement() : expression(NULL) {}
    virtual ~Statement() { delete expression; }
    virtual Term* createTerm(CompilationContext &context) = 0;
    virtual std::string typeName() const { return "Statement"; }
    virtual int numChildren() const { return 1; }
    virtual ASTNode* getChild(int index) const
    {
        if (index == 0) return expression;
        return NULL; 
    }

    typedef std::vector<Statement*> Vector;
};

struct StatementList : public ASTNode
{
    Statement::Vector statements;

    void push(Statement* statement);

    virtual ~StatementList();
    void createTerms(CompilationContext &context);
    virtual std::string typeName() const { return "StatementList"; }
    virtual int numChildren() const { return statements.size(); }
    virtual ASTNode* getChild(int index) const { return statements[index]; }
    int count() const { return (int) statements.size(); }
    Statement* operator[](int index) { return statements[index]; }
};

struct ExpressionStatement : public Statement
{
    std::string nameBinding;
    std::string preEqualsWhitepace;
    std::string postEqualsWhitespace;

    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "ExpressionStatement"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

// 'CommentStatement' includes comments and blank lines
struct CommentStatement : public Statement
{
    std::string text;

    CommentStatement(std::string const& _text) : text(_text) {}
    virtual ~CommentStatement() { }
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "CommentStatement"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
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
};

struct FunctionDecl : public Statement
{
    FunctionHeader *header;
    StatementList *statements;

    FunctionDecl() : header(NULL), statements(NULL) {}
    ~FunctionDecl();

    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "FunctionDecl"; }
    virtual int numChildren() const { return 1; }
    virtual ASTNode* getChild(int index) const { return statements; }
};

struct TypeDecl : public Statement
{
    struct Member {
        std::string type;
        std::string name;

        Member(std::string const& _type, std::string const& _name)
          : type(_type), name(_name) {}
    };
    typedef std::vector<Member> MemberList;

    std::string name;
    MemberList members;

    void addMember(std::string const& type, std::string const& name)
    {
        members.push_back(Member(type, name));
    }

    // for Statement
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "TypeDecl"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct IfStatement : public Statement
{
    Expression* condition;
    StatementList* positiveBranch;
    StatementList* negativeBranch;

    IfStatement() : condition(NULL), positiveBranch(NULL), negativeBranch(NULL) {}
    ~IfStatement();

    // for Statement
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "IfStatement"; }
    virtual int numChildren() const { return 0; }
    virtual ASTNode* getChild(int index) const { return NULL; }
};

struct StatefulValueDeclaration : public Statement
{
    std::string type;
    std::string name;
    Expression* initialValue;

    StatefulValueDeclaration() : initialValue(NULL) {}

    // for Statement
    virtual Term* createTerm(CompilationContext &context);
    virtual std::string typeName() const { return "StatefulValueDeclaration"; }
    virtual int numChildren() const { return 1; }
    virtual ASTNode* getChild(int index) const { return initialValue; }
};

std::string print_ast(ASTNode *node);

} // namespace ast
} // namespace circa

#endif
