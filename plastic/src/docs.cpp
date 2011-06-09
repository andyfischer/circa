// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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
    append_package_docs(out, nested_contents(runtimeBranch["box2d"]), "box2d bindings");
    out << ",";
    append_package_docs(out, nested_contents(runtimeBranch["gl"]), "gl namespace");
    out << ",";
    append_package_docs(out, nested_contents(runtimeBranch["ide"]), "ide namespace");
    out << ",";
    append_package_docs(out, nested_contents(runtimeBranch["image"]), "image namespace");
    out << ",";
    append_package_docs(out, nested_contents(runtimeBranch["text"]), "text namespace");
    out << ",";
    append_package_docs(out, nested_contents(runtimeBranch["tweak"]), "tweak namespace");

    out << "]\n";
    out << "}\n";
}
