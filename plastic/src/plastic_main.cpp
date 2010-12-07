// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "display.h"
#include "docs.h"
#include "gl_shapes.h"
#include "gl_util.h"
#include "ide.h"
#include "image.h"
#include "input.h"
#include "mesh.h"
#include "postprocess.h"
#include "text.h"
#include "textures.h"

#include "plastic_main.h"

using namespace circa;

void main_loop()
{
    input::capture_events();

    long ticks = SDL_GetTicks();
    static long previousTicks = SDL_GetTicks();
    long ticksAdvanced = ticks - previousTicks;
    previousTicks = ticks;

    // Evaluate script
    if (!app::paused()) {
        app::singleton()._ticksElapsed += ticksAdvanced;
    }

    gl_clear_error();

    render_frame();

    long new_ticks = SDL_GetTicks();

    // Delay to limit framerate
    const long ticks_per_second = long(1.0 / app::singleton()._targetFps * 1000);
    if ((new_ticks - ticks) < ticks_per_second) {
        long delay = ticks_per_second - (new_ticks - ticks);
        SDL_Delay(delay);
    }
}

int plastic_main(std::vector<std::string> args)
{
    if (!app::initialize()) return 1;

    std::string arg0;
    if (args.size() > 0) arg0 = args[0];

    // -gd to generate docs
    if (arg0 == "-gd") {
        std::cout << "writing docs to " << args[1] << std::endl;
        std::stringstream out;
        generate_plastic_docs(out);
        circa::storage::write_text_file(args[1].c_str(), out.str().c_str());
        return 0;
    }

    // -p to print raw compiled code
    if (arg0 == "-p") {
        if (!app::load_user_script_filename(args[1]))
            return 1;

        EvalContext cxt;
        include_function::preload_script(app::users_branch().owningTerm);

        print_branch(std::cout, app::users_branch());
        return 0;
    }

    // -tr to do a test run of a script, without creating a display.
    if (arg0 == "-tr") {
        if (!app::load_user_script_filename(args[1]))
            return 1;

        if (has_static_errors(app::users_branch())) {
            print_static_errors_formatted(app::users_branch(), std::cout);
            return 1;
        }

        for (int i=0; i < 5; i++)
            if (!app::evaluate_main_script())
                return 1;

        return 0;
    }

    // Normal operation, load the script file in argument 0.
    std::string filename = arg0;

    if (!app::load_user_script_filename(filename))
        return 1;

    if (has_static_errors(app::users_branch())) {
        print_static_errors_formatted(app::users_branch(), std::cout);
        return 1;
    }

    if (!initialize_display()) return 1;

    // Main loop
    while (app::continue_main_loop())
        main_loop();

    // Quit SDL
    SDL_Quit();

    return 0;
}
