
#include "common_headers.h"

#include "ast.h"

namespace circa {
namespace ast {

Infix::Infix()
  : left(NULL), right(NULL)
{
}

Infix::~Infix()
{
    delete left;
    delete right;
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

void
StatementList::push(Statement* statement)
{
    this->statements.push_back(statement);
}




} // namespace ast
} // namespace circa
