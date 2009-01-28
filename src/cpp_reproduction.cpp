// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

std::string type_id_to_cpp(Type& type)
{
    if (type.cppTypeName != "")
        return type.cppTypeName;
    else
        return type.name;
}

std::string function_decl_to_cpp(Function& func)
{
    std::stringstream out;

    out << type_id_to_cpp(as_type(func.outputType)) << func.name << "(\n";

    for (unsigned int i=0; i < func.inputTypes.count(); i++) {
        Term* type = func.inputTypes[i];
        std::string name = func.inputProperties[i].name;
        if (i > 0) out << ", ";
        out << type_id_to_cpp(as_type(type)) << " " << name;
    }

    out << ")\n";
    out << "{\n";
    // TODO
    out << "}\n";

    return out.str();
}

} // namespace circa
