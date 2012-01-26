// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

void read_stdin_line(TValue* line)
{
    char* buf = NULL;
    size_t size = 0;
    ssize_t read = getline(&buf, &size, stdin);

    if (read == -1) {
        set_null(line);
        free(buf);
        return;
    }

    buf[read] = 0;

    // Truncate newline
    if (read > 0 && buf[read-1] == '\n')
        buf[read - 1] = 0;

    set_string(line, buf);
    free(buf);
}

void parse_argument_list(TValue* string, List* out)
{
}

void run_command_reader()
{
    std::cout << "started command reader" << std::endl;
    while (true) {
        TValue line;
        read_stdin_line(&line);
        std::cout << "read line: " << line.toString() << std::endl;
        if (!is_string(&line))
            break;
    }
}

} // namespace circa
