// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Branch& branch = as_branch(caller);
        evaluate_branch(cxt, branch);
    }

    std::string toSourceString(Term* term) {

        std::stringstream out;

        if (term->boolPropOptional("syntax:literal-list", false)) {
            prepend_name_binding(term, out);
            out << "[";
            Branch& contents = as_branch(term);
            out << get_branch_source(contents, "");
            out << "]";
        } else if (term->type == NAMESPACE_TYPE) {
            out << "namespace ";
            out << term->name;
            out << term->stringPropOptional("syntax:postHeadingWs", "\n");
            out << get_branch_source(as_branch(term));
            out << term->stringPropOptional("syntax:preEndWs", "");
            out << "end";
            
        } else {
            prepend_name_binding(term, out);
            print_branch_source(out, term);
        }
        return out.str();
    }

    void format_source(RichSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:literal-list", false)) {
            append_leading_name_binding(source, term);
            append_phrase(source, "[", term, token::LBRACKET);
            Branch& contents = as_branch(term);

            for (int i=0; i < contents.length(); i++) {
                Term* term = contents[i];
                append_term_source(source, term);
            }

            append_phrase(source, "]", term, token::RBRACKET);
        } else if (term->type == NAMESPACE_TYPE) {
#if 0
            out << "namespace ";
            out << term->name;
            out << term->stringPropOptional("syntax:postHeadingWs", "\n");
            out << get_branch_source(as_branch(term));
            out << term->stringPropOptional("syntax:preEndWs", "");
            out << "end";
#endif
            
        } else {
#if 0
            prepend_name_binding(term, out);
            print_branch_source(out, term);
#endif
        }
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, evaluate, "branch() -> Branch");
        function_t::get_attrs(BRANCH_FUNC).toSource = toSourceString;
        function_t::get_attrs(BRANCH_FUNC).formatSource = format_source;
    }
}
}
