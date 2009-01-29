// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

std::string type_id_to_cpp(Type& type)
{
    if (type.cppTypeName != "")
        return type.cppTypeName;
    else
        return type.name;
}

std::string statement_to_cpp(Term* term)
{
    std::stringstream out;

    // special cases
    if (term->function == COMMENT_FUNC) {
        std::string comment = get_comment_string(term);
        if (comment == "")
            out << std::endl;
        else
            out << "//" << comment << std::endl;

        return out.str();
    }

    if (is_value(term)) {
        out << type_id_to_cpp(as_type(term->type)) << " " << term->name << ";";
        return out.str();
    }

    if (term->name != "") {
        out << type_id_to_cpp(as_type(term->type)) << " " << term->name << " = ";
    }

    out << term->syntaxHints.functionName << "(";

    for (int i=0; i < term->numInputs(); i++) {
        if (i > 0) out << ", ";
        out << get_source_of_input(term, i);
    }

    out << ");";

    return out.str();
}

std::string function_decl_to_cpp(Function& func)
{
    std::stringstream out;

    out << type_id_to_cpp(as_type(func.outputType)) << " " << func.name << "(";

    for (unsigned int i=0; i < func.inputTypes.count(); i++) {
        Term* type = func.inputTypes[i];
        std::string name = func.inputProperties[i].name;
        if (i > 0) out << ", ";
        out << type_id_to_cpp(as_type(type)) << " " << name;
    }

    out << ")\n";
    out << "{\n";

    for (int i=0; i < func.subroutineBranch.numTerms(); i++) {
        Term* term = func.subroutineBranch[i];
        if (!should_print_term_source_line(term))
            continue;

        out << "   " << statement_to_cpp(term) << std::endl;
    }

    out << "}\n";

    return out.str();
}

} // namespace circa
