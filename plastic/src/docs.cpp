// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include "app.h"
#include "plastic_main.h"

using namespace circa;

void generate_plastic_docs(std::stringstream &out)
{
    // Header information
    out << "{\n";
    out << "  \"headers\": { \"title\": \"Circa+Plastic\" },\n";
    out << "  \"packages\": [\n";

    Branch& runtimeBranch = app::runtime_branch();

    append_package_docs(out, *KERNEL, "Circa builtins");
    out << ",";
    append_package_docs(out, runtimeBranch, "Plastic top level");
    out << ",";
    append_package_docs(out, runtimeBranch["gl"]->asBranch(), "gl namespace");
    out << ",";
    append_package_docs(out, runtimeBranch["image"]->asBranch(), "image namespace");
    out << ",";
    append_package_docs(out, runtimeBranch["text"]->asBranch(), "text namespace");
    out << ",";
    append_package_docs(out, runtimeBranch["tweak"]->asBranch(), "tweak namespace");

    out << "]\n";
    out << "}\n";
}
