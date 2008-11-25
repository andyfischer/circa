// Generated file

#include "add.cpp"
#include "and.cpp"
#include "concat.cpp"
#include "equals.cpp"
#include "if-expr.cpp"
#include "list-append.cpp"
#include "list-apply.cpp"
#include "list-pack.cpp"
#include "map.cpp"
#include "mult.cpp"
#include "or.cpp"
#include "parse-expression.cpp"
#include "parse-function-header.cpp"
#include "print.cpp"
#include "range.cpp"
#include "read-text-file.cpp"
#include "to-string.cpp"
#include "tokenize.cpp"
#include "write-text-file.cpp"

void setup_generated_functions(Branch& kernel)
{
    // do parsing functions first
    parse_function_header_function::setup(kernel);
    tokenize_function::setup(kernel);

    add_function::setup(kernel);
    and_function::setup(kernel);
    concat_function::setup(kernel);
    equals_function::setup(kernel);
    if_expr_function::setup(kernel);
    list_append_function::setup(kernel);
    list_apply_function::setup(kernel);
    list_pack_function::setup(kernel);
    map_function::setup(kernel);
    mult_function::setup(kernel);
    or_function::setup(kernel);
    parse_expression_function::setup(kernel);
    print_function::setup(kernel);
    range_function::setup(kernel);
    read_text_file_function::setup(kernel);
    to_string_function::setup(kernel);
    write_text_file_function::setup(kernel);
}
