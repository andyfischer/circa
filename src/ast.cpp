
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
FunctionCall::addArgument(std::string const& preWhitespace, Expression* expr,
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

void
StatementList::push(Statement* statement)
{
    this->statements.push_back(statement);
}


} // namespace ast
} // namespace circa
