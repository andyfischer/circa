// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "block.h"
#include "kernel.h"
#include "names.h"
#include "term.h"
#include "type.h"

namespace circa {

std::string get_cpp_type_name(Type* type)
{
    return name_to_string(type->name);
}

std::string get_cpp_type_accessor(Type* type)
{
    if (type == TYPES.int_type) return "as_int";
    else if (type == TYPES.float_type) return "as_float";
    else if (type == TYPES.string) return "as_string";
    else if (type == TYPES.bool_type) return "as_bool";
    else return "(unknown accessor)";
}

std::string cpp_accessor_for_type(Type* type)
{
#if 0
    std::stringstream out;
    std::string indent = "    ";

    out << "struct " << get_cpp_type_name(type) << "\n";
    out << "{\n";
    out << indent << "Term* _term\n";
    out << "\n";
    out << indent << get_cpp_type_name(type) << "(Term* term) : _term(term) {}\n";
    out << "\n";

    Block* prototype = type->prototype;
    for (int i=0; i < prototype.length(); i++) {
        Term* field = prototype[i];
        out << indent << get_cpp_type_name(field->type) << "& " << field->name << "() ";
        out << "{ return " << get_cpp_type_accessor(field->type) << "(_term->field(" <<
            i << ")); }\n";
    }

    out << "};\n";
    return out.str();
#endif
    return "";
}

std::string generate_cpp_headers(Block* block)
{
    std::stringstream out;

    for (int i=0; i < block->length(); i++) {
        if (block->get(i) == NULL) continue;

        Term* term = block->get(i);

        if (is_type(term))
            out << cpp_accessor_for_type(as_type(term_value(term)));
    }

    return out.str();
}

} // namespace circa
