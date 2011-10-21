// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

namespace circa {
namespace cpp_codegen {

struct CppWriter
{
    int indentLevel;
    std::stringstream strm;
    bool needsIndent;

    CppWriter() : indentLevel(0), needsIndent(false) {}

    void indent() {
        indentLevel++;
    }
    void unindent() {
        indentLevel--;
    }

    void newline() {
        strm << "\n";
        needsIndent = true;
    }
    void write(const std::string& str)
    {
        if (needsIndent) {
            for (int i=0; i < indentLevel; i++)
                strm << "    ";
            needsIndent = false;
        }
        strm << str;
    }

    std::string result() { return strm.str(); }
};

void write_term(CppWriter& writer, Term* term);
void write_branch_contents(CppWriter& writer, Branch* branch);

} // namespace cpp_codegen
} // namespace circa
