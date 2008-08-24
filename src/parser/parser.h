#ifndef CIRCA__PARSER__INCLUDED
#define CIRCA__PARSER__INCLUDED

#include "ast.h"
#include "token_stream.h"

namespace circa {
namespace parser {

ast::StatementList* statementList(TokenStream* tokens);
ast::Statement* statement(TokenStream* tokens);
ast::Expression* infixExpression(TokenStream* tokens);
ast::Expression* atom(TokenStream* tokens);
ast::FunctionCall* functionCall(TokenStream* tokens);
ast::LiteralString* literalString(TokenStream* tokens);
ast::LiteralFloat* literalFloat(TokenStream* tokens);
ast::Identifier* identifier(TokenStream* tokens);

} // namespace parser
} // namespace circa

#endif
