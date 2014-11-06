
#pragma once

namespace improv { class App; }

extern "C" {
    int sdl_window_main_loop(improv::App* app);
}
