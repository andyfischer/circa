// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../block.h"
#include "../inspection.h"
#include "../static_checking.h"
#include "../term.h"

namespace circa {

int run_exporting_parser(const char* format, const char* filename)
{
    if (strcmp(format, "") == 0 || strcmp(filename, "") == 0) {
        std::cout << "usage: -export <format> <filename>" << std::endl;
        return -1;
    }

    Block block;
    load_script(&block, filename);

    Value str;
    if (print_static_errors_formatted(&block, &str)) {
        dump(&str);
        return -1;
    }

    if (strcmp(format, "json") == 0) {
        std::cout << "{\n";
        bool needsComma = false;
        for (int i=0; i < block.length(); i++) {
            Term* term = block[i];
            if (term->name == "" || is_hidden(term))
                continue;

            if (needsComma)
                std::cout << ",\n";

            std::cout << "'" << term->name << "': ";

            caValue* value = term_value(term);
            if (is_int(value) || is_float(value) || is_bool(value)) {
                Value str;
                to_string(value, &str);
                std::cout << as_cstring(&str);
            } else
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
