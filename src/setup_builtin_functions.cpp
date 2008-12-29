
// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"

namespace circa {

namespace add_function { void setup(Branch& kernel); }
namespace and_function { void setup(Branch& kernel); }
namespace apply_feedback_function { void setup(Branch& kernel); }
namespace assert_function { void setup(Branch& kernel); }
namespace concat_function { void setup(Branch& kernel); }
namespace cos_function { void setup(Branch& kernel); }
namespace div_function { void setup(Branch& kernel); }
namespace equals_function { void setup(Branch& kernel); }
namespace evaluate_file_function { void setup(Branch& kernel); }
namespace function_get_input_name_function { void setup(Branch& kernel); }
namespace function_name_input_function { void setup(Branch& kernel); }
namespace get_branch_bound_names_function { void setup(Branch& kernel); }
namespace get_infix_function_name_function { void setup(Branch& kernel); }
namespace greater_than_function { void setup(Branch& kernel); }
namespace if_expr_function { void setup(Branch& kernel); }
namespace if_statement_function { void setup(Branch& kernel); }
namespace less_than_function { void setup(Branch& kernel); }
namespace list_append_function { void setup(Branch& kernel); }
namespace list_apply_function { void setup(Branch& kernel); }
namespace list_function { void setup(Branch& kernel); }
namespace map_function { void setup(Branch& kernel); }
namespace mult_function { void setup(Branch& kernel); }
namespace not_function { void setup(Branch& kernel); }
namespace or_function { void setup(Branch& kernel); }
namespace parse_expression_function { void setup(Branch& kernel); }
namespace parse_function_header_function { void setup(Branch& kernel); }
namespace print_function { void setup(Branch& kernel); }
namespace range_function { void setup(Branch& kernel); }
namespace read_text_file_function { void setup(Branch& kernel); }
namespace set_function { void setup(Branch& kernel); }
namespace set_union_function { void setup(Branch& kernel); }
namespace sin_function { void setup(Branch& kernel); }
namespace sub_function { void setup(Branch& kernel); }
namespace subroutine_apply_function { void setup(Branch& kernel); }
namespace subroutine_create_function { void setup(Branch& kernel); }
namespace to_string_function { void setup(Branch& kernel); }
namespace tokenize_function { void setup(Branch& kernel); }
namespace tuple_function { void setup(Branch& kernel); }
namespace write_text_file_function { void setup(Branch& kernel); }

void setup_builtin_functions(Branch& kernel)
{
    add_function::setup(kernel);
    and_function::setup(kernel);
    apply_feedback_function::setup(kernel);
    assert_function::setup(kernel);
    concat_function::setup(kernel);
    cos_function::setup(kernel);
    div_function::setup(kernel);
    equals_function::setup(kernel);
    evaluate_file_function::setup(kernel);
    function_get_input_name_function::setup(kernel);
    function_name_input_function::setup(kernel);
    get_branch_bound_names_function::setup(kernel);
    get_infix_function_name_function::setup(kernel);
    greater_than_function::setup(kernel);
    if_expr_function::setup(kernel);
    if_statement_function::setup(kernel);
    less_than_function::setup(kernel);
    list_append_function::setup(kernel);
    list_apply_function::setup(kernel);
    list_function::setup(kernel);
    map_function::setup(kernel);
    mult_function::setup(kernel);
    not_function::setup(kernel);
    or_function::setup(kernel);
    parse_expression_function::setup(kernel);
    parse_function_header_function::setup(kernel);
    print_function::setup(kernel);
    range_function::setup(kernel);
    read_text_file_function::setup(kernel);
    set_function::setup(kernel);
    set_union_function::setup(kernel);
    sin_function::setup(kernel);
    sub_function::setup(kernel);
    subroutine_apply_function::setup(kernel);
    subroutine_create_function::setup(kernel);
    to_string_function::setup(kernel);
    tokenize_function::setup(kernel);
    tuple_function::setup(kernel);
    write_text_file_function::setup(kernel);
}

} // namespace circa
