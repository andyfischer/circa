// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "introspection.h"
#include "static_checking.h"
#include "term.h"

namespace circa {

int run_exporting_parser(const char* format, const char* filename)
{
    if (strcmp(format, "") == 0 || strcmp(filename, "") == 0) {
        std::cout << "usage: -export <format> <filename>" << std::endl;
        return -1;
    }

    Branch branch;
    load_script(&branch, filename);

    if (print_static_errors_formatted(branch, std::cout))
        return -1;

    if (strcmp(format, "json") == 0) {
        std::cout << "{\n";
        bool needsComma = false;
        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            if (term->name == "" || is_hidden(term))
                continue;

            if (needsComma)
                std::cout << ",\n";

            std::cout << "'" << term->name << "': ";

            TaggedValue* value = term;
            if (is_int(value) || is_float(value) || is_bool(value))
                std::cout << to_string(value);
            else
                std::cout << "null";

            needsComma = true;
        }

        std::cout << "\n}" << std::endl;
        return 0;

    } else {
        std::cout << "unrecognized format: " << format << std::endl;
        return -1;
    }
}

} // namespace circa
