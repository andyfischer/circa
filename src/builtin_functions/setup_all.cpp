// Generated file

#include "add.cpp"
#include "and.cpp"
#include "concat.cpp"
#include "if-expr.cpp"
#include "list-append.cpp"
#include "list-apply.cpp"
#include "mult.cpp"
#include "or.cpp"
#include "print.cpp"
#include "range.cpp"
#include "read-text-file.cpp"
#include "to-string.cpp"
#include "tokenize.cpp"
#include "write-text-file.cpp"

void setup_generated_functions(Branch& kernel)
{
    add_function::setup(kernel);
    and_function::setup(kernel);
    concat_function::setup(kernel);
    if_expr_function::setup(kernel);
    list_append_function::setup(kernel);
    list_apply_function::setup(kernel);
    mult_function::setup(kernel);
    or_function::setup(kernel);
    print_function::setup(kernel);
    range_function::setup(kernel);
    read_text_file_function::setup(kernel);
    to_string_function::setup(kernel);
    tokenize_function::setup(kernel);
    write_text_file_function::setup(kernel);
}
