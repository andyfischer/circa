
// Copyright 2008 Paul Hodge

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
namespace if_expr_function { void setup(Branch& kernel); }
namespace list_append_function { void setup(Branch& kernel); }
namespace list_apply_function { void setup(Branch& kernel); }
namespace list_pack_function { void setup(Branch& kernel); }
namespace map_function { void setup(Branch& kernel); }
namespace mult_function { void setup(Branch& kernel); }
namespace or_function { void setup(Branch& kernel); }
namespace parse_expression_function { void setup(Branch& kernel); }
namespace parse_function_header_function { void setup(Branch& kernel); }
namespace print_function { void setup(Branch& kernel); }
namespace range_function { void setup(Branch& kernel); }
namespace read_text_file_function { void setup(Branch& kernel); }
namespace sin_function { void setup(Branch& kernel); }
namespace sub_function { void setup(Branch& kernel); }
namespace to_string_function { void setup(Branch& kernel); }
namespace tokenize_function { void setup(Branch& kernel); }
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
    if_expr_function::setup(kernel);
    list_append_function::setup(kernel);
    list_apply_function::setup(kernel);
    list_pack_function::setup(kernel);
    map_function::setup(kernel);
    mult_function::setup(kernel);
    or_function::setup(kernel);
    parse_expression_function::setup(kernel);
    parse_function_header_function::setup(kernel);
    print_function::setup(kernel);
    range_function::setup(kernel);
    read_text_file_function::setup(kernel);
    sin_function::setup(kernel);
    sub_function::setup(kernel);
    to_string_function::setup(kernel);
    tokenize_function::setup(kernel);
    write_text_file_function::setup(kernel);
}

} // namespace circa
