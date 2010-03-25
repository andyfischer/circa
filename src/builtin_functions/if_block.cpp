// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_block_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        evaluate_if_block(cxt, caller);
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;

        Branch& contents = as_branch(term);

        for (int branch_index=0; branch_index < contents.length(); branch_index++) {
            Term* branch_term = contents[branch_index];

            if (is_hidden(branch_term))
                continue;

            result << branch_term->stringPropOptional("syntax:preWhitespace", "");

            if (branch_index == 0) {
                result << "if ";
                result << get_source_of_input(branch_term, 0);
            } else if (branch_index < (contents.length()-2)) {
                result << "elif ";
                result << get_source_of_input(branch_term, 0);
            }
            else
                result << "else";

            result << get_branch_source(as_branch(branch_term));
        }

        result << term->stringPropOptional("syntax:whitespaceBeforeEnd", "");
        result << "end";

        return result.str();
    }

    void formatSource(RichSource* source, Term* term)
    {
        Branch& contents = as_branch(term);

        for (int branch_index=0; branch_index < contents.length(); branch_index++) {
            Term* branch_term = contents[branch_index];

            if (is_hidden(branch_term))
                continue;

            append_phrase(source,
                    branch_term->stringPropOptional("syntax:preWhitespace", ""),
                    branch_term, token::WHITESPACE);

            if (branch_index == 0) {
                append_phrase(source, "if ", branch_term, phrase_type::KEYWORD);
                append_source_for_input(source, branch_term, 0);
            } else if (branch_index < (contents.length()-2)) {
                append_phrase(source, "elif ", branch_term, phrase_type::KEYWORD);
                append_source_for_input(source, branch_term, 0);
            }
            else
                append_phrase(source, "else", branch_term, phrase_type::UNDEFINED);

            append_branch_source(source, as_branch(branch_term), NULL);
        }

        append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
                term, token::WHITESPACE);
        append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
        IF_BLOCK_FUNC = import_function(kernel, evaluate, "if_block(List _state) -> Branch");
        function_t::get_attrs(IF_BLOCK_FUNC).toSource = toSourceString;
        function_t::get_attrs(IF_BLOCK_FUNC).formatSource = formatSource;
        function_t::set_input_meta(IF_BLOCK_FUNC, 0, true); // allow _state to be NULL
        function_t::set_exposed_name_path(IF_BLOCK_FUNC, "#joining");
    }
}
}
