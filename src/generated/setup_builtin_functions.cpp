
// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file is generated during the build process by ca-prebuild.py .
// You should probably not edit this file manually.

#include "../common_headers.h"

#include "../block.h"

namespace circa {

namespace add_function { void setup(Block* kernel); }
namespace block_function { void setup(Block* kernel); }
namespace cast_function { void setup(Block* kernel); }
namespace comment_function { void setup(Block* kernel); }
namespace comparison_function { void setup(Block* kernel); }
namespace concat_function { void setup(Block* kernel); }
namespace cond_function { void setup(Block* kernel); }
namespace copy_function { void setup(Block* kernel); }
namespace div_function { void setup(Block* kernel); }
namespace extra_output_function { void setup(Block* kernel); }
namespace for_function { void setup(Block* kernel); }
namespace get_field_function { void setup(Block* kernel); }
namespace get_index_function { void setup(Block* kernel); }
namespace if_block_function { void setup(Block* kernel); }
namespace increment_function { void setup(Block* kernel); }
namespace input_explicit_function { void setup(Block* kernel); }
namespace internal_debug_function { void setup(Block* kernel); }
namespace list_function { void setup(Block* kernel); }
namespace logical_function { void setup(Block* kernel); }
namespace make_function { void setup(Block* kernel); }
namespace math_function { void setup(Block* kernel); }
namespace mult_function { void setup(Block* kernel); }
namespace neg_function { void setup(Block* kernel); }
namespace rand_function { void setup(Block* kernel); }
namespace range_function { void setup(Block* kernel); }
namespace rpath_function { void setup(Block* kernel); }
namespace set_field_function { void setup(Block* kernel); }
namespace set_index_function { void setup(Block* kernel); }
namespace static_error_function { void setup(Block* kernel); }
namespace sub_function { void setup(Block* kernel); }
namespace switch_function { void setup(Block* kernel); }
namespace term_to_source_function { void setup(Block* kernel); }
namespace trig_function { void setup(Block* kernel); }
namespace type_check_function { void setup(Block* kernel); }
namespace unique_id_function { void setup(Block* kernel); }
namespace unknown_function_function { void setup(Block* kernel); }
namespace unknown_identifier_function { void setup(Block* kernel); }
namespace unrecognized_expr_function { void setup(Block* kernel); }
namespace write_text_file_function { void setup(Block* kernel); }

void setup_builtin_functions(Block* kernel)
{
    add_function::setup(kernel);
    block_function::setup(kernel);
    cast_function::setup(kernel);
    comment_function::setup(kernel);
    comparison_function::setup(kernel);
    concat_function::setup(kernel);
    cond_function::setup(kernel);
    copy_function::setup(kernel);
    div_function::setup(kernel);
    extra_output_function::setup(kernel);
    for_function::setup(kernel);
    get_field_function::setup(kernel);
    get_index_function::setup(kernel);
    if_block_function::setup(kernel);
    increment_function::setup(kernel);
    input_explicit_function::setup(kernel);
    internal_debug_function::setup(kernel);
    list_function::setup(kernel);
    logical_function::setup(kernel);
    make_function::setup(kernel);
    math_function::setup(kernel);
    mult_function::setup(kernel);
    neg_function::setup(kernel);
    rand_function::setup(kernel);
    range_function::setup(kernel);
    rpath_function::setup(kernel);
    set_field_function::setup(kernel);
    set_index_function::setup(kernel);
    static_error_function::setup(kernel);
    sub_function::setup(kernel);
    switch_function::setup(kernel);
    term_to_source_function::setup(kernel);
    trig_function::setup(kernel);
    type_check_function::setup(kernel);
    unique_id_function::setup(kernel);
    unknown_function_function::setup(kernel);
    unknown_identifier_function::setup(kernel);
    unrecognized_expr_function::setup(kernel);
    write_text_file_function::setup(kernel);
}

} // namespace circa
