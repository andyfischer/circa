// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

#include "plastic.h"

using namespace circa;

void generate_plastic_docs(std::stringstream &out)
{
    // Header information
    out << "{\n";
    out << "  \"headers\": { \"title\": \"Circa+Plastic\" },\n";
    out << "  \"packages\": [\n";

    append_package_docs(out, *KERNEL, "Circa builtins");
    out << ",";
    append_package_docs(out, runtime_branch(), "Plastic top level");
    out << ",";
    append_package_docs(out, runtime_branch()["gl"]->asBranch(), "gl namespace");
    out << ",";
    append_package_docs(out, runtime_branch()["text"]->asBranch(), "text namespace");
    out << ",";
    append_package_docs(out, runtime_branch()["tweak"]->asBranch(), "tweak namespace");

    out << "]\n";
    out << "}\n";
}
