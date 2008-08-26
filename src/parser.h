#ifndef CIRCA__PARSER__INCLUDED
#define CIRCA__PARSER__INCLUDED

#include "ast.h"
#include "token_stream.h"

namespace circa {
namespace parser {

Term* quick_eval_statement(Branch* branch, std::string const& input);
Term* quick_exec_statement(Branch* branch, std::string const& input);

ast::StatementList* statementList(token_stream::TokenStream& tokens);
ast::Statement* statement(token_stream::TokenStream& tokens);
ast::Expression* infixExpression(token_stream::TokenStream& tokens);
ast::Expression* atom(token_stream::TokenStream& tokens);
ast::FunctionCall* functionCall(token_stream::TokenStream& tokens);
ast::LiteralString* literalString(token_stream::TokenStream& tokens);
ast::LiteralFloat* literalFloat(token_stream::TokenStream& tokens);
ast::Identifier* identifier(token_stream::TokenStream& tokens);

} // namespace parser
} // namespace circa

#endif
