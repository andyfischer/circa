// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "app.h"
#include "gl_util.h"

namespace platform_ipad {
    
void setup_gl()
{
}

void render()
{
    gl_clear_error();
    app::evaluate_main_script();

    const char* err = gl_check_error();
    if (err != NULL) {
        std::cerr << "Uncaught GL error (in render_frame): " << err << std::endl;
    }
}

} // namespace platform_ipad
