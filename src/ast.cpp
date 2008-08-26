
#include "common_headers.h"

#include "circa.h"

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

Term*
FunctionCall::createTerm(Branch* branch)
{
    Term* function = find_named(branch, this->functionName);

    TermList inputs;

    ArgumentList::const_iterator it;
    for (it = this->arguments.begin(); it != this->arguments.end(); ++it) {
        Term* term = (*it)->expression->createTerm(branch);
        assert(term != NULL);
        inputs.append(term);
    }

    return apply_function(branch, function, inputs);
}

std::string
LiteralString::toString() const
{
    return "\"" + this->text + "\"";
}

Term*
LiteralString::createTerm(Branch* branch)
{
    return constant_string(branch, this->text);
}

Term*
LiteralFloat::createTerm(Branch* branch)
{
    float value = atof(this->text.c_str());
    return constant_float(branch, value);
}

Term*
LiteralInteger::createTerm(Branch* branch)
{
    int value = atoi(this->text.c_str());
    return constant_int(branch, value);
}

std::string
Identifier::toString() const
{
    return text;
}
Term*
Identifier::createTerm(Branch* branch)
{
    return find_named(branch,this->text);
}

std::string
ExpressionStatement::toString() const
{
    std::string output;

    if (nameBinding != "") {
        output = nameBinding + preEqualsWhitepace + "=" + postEqualsWhitespace;
    }

    output += expression->toString();
    
    return output;
}

Term*
ExpressionStatement::createTerm(Branch* branch)
{
    Term* term = this->expression->createTerm(branch);
    
    if (this->nameBinding != "")
        branch->bindName(term, this->nameBinding);

    return term;
}

void
StatementList::push(Statement* statement)
{
    this->statements.push_back(statement);
}

std::string
StatementList::toString() const
{
    std::stringstream output;

    Statement::List::const_iterator it;
    for (it = statements.begin(); it != statements.end(); ++it) {
        output << (*it)->toString() << "\n";
    }
    return output.str();
}

} // namespace ast
} // namespace circa
