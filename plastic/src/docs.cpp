// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"
#include "main.h"

using namespace circa;

void generate_plastic_docs(std::stringstream &out)
{
    // Header information
    out << "{\n";
    out << "  \"headers\": { \"title\": \"Circa+Plastic\" },\n";
    out << "  \"packages\": [\n";

    append_package_docs(out, *KERNEL, "Circa builtins");
    out << ",";
    append_package_docs(out, *SCRIPT_ROOT, "Plastic top level");
    out << ",";
    append_package_docs(out, SCRIPT_ROOT->get("gl")->asBranch(), "gl namespace");
    out << ",";
    append_package_docs(out, SCRIPT_ROOT->get("text")->asBranch(), "text namespace");
    out << ",";
    append_package_docs(out, SCRIPT_ROOT->get("tweak")->asBranch(), "tweak namespace");

    out << "]\n";
    out << "}\n";
}
