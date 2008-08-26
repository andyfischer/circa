
#include "common_headers.h"
#include "circa.h"

namespace circa {
namespace syntax_errors {

SyntaxError::SyntaxError(std::string const& message,
        tokenizer::TokenInstance const* location) throw()
{
    this->msg = "SyntaxError: " + message;

    if (location != NULL) {
        std::stringstream text;
        text << ", at line " << location->line << ", char " << location->character;
        this->msg += text.str();
    }
}

} // namespace circa
} // namespace syntax_errors
