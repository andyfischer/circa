// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

std::string get_cpp_type_name(Term* type)
{
    return type->name;
}

std::string get_cpp_type_accessor(Term* type)
{
    if (type == INT_TYPE) return "as_int";
    else if (type == FLOAT_TYPE) return "as_float";
    else if (type == STRING_TYPE) return "as_string";
    else if (type == BOOL_TYPE) return "as_bool";
    else return "(unknown accessor)";
}

std::string cpp_accessor_for_type(Term* term)
{
    std::stringstream out;
    std::string indent = "    ";

    out << "struct " << get_cpp_type_name(term) << "\n";
    out << "{\n";
    out << indent << "Term* _term\n";
    out << "\n";
    out << indent << get_cpp_type_name(term) << "(Term* term) : _term(term) {}\n";
    out << "\n";

    Branch& prototype = type_t::get_prototype(term);
    for (int i=0; i < prototype.length(); i++) {
        Term* field = prototype[i];
        out << indent << get_cpp_type_name(field->type) << "& " << field->name << "() ";
        out << "{ return " << get_cpp_type_accessor(field->type) << "(_term->field(" <<
            i << ")); }\n";
    }

    out << "};\n";
    return out.str();
}

std::string generate_cpp_headers(Branch& branch)
{
    std::stringstream out;

    for (int i=0; i < branch.length(); i++) {
        if (branch[i] == NULL) continue;

        Term* term = branch[i];

        if (is_type(term))
            out << cpp_accessor_for_type(term);
    }

    return out.str();
}

} // namespace circa
