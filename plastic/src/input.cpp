// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <importing_macros.h>
#include <circa.h>

#include "plastic_common_headers.h"

#include "app.h"
#include "display.h"
#include "gl_util.h"
#include "plastic_main.h"
#include "pause_status.h"

#include "input.h"

using namespace circa;

namespace input {

bool KEY_DOWN[SDLK_LAST];
std::vector<SDL_keysym> KEYS_JUST_PRESSED;

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool LEFT_MOUSE_DOWN = false;
bool RECENT_LEFT_MOUSE_DOWN = false;
bool RECENT_MOUSE_WHEEL_UP = false;
bool RECENT_MOUSE_WHEEL_DOWN = false;

Ref MOUSE_POSITION_TERM;

void handle_key_press(SDL_Event &event, int key);

CA_FUNCTION(key_down)
{
    int i = INT_INPUT(0);
    set_bool(OUTPUT, KEY_DOWN[i]);
}

CA_FUNCTION(key_pressed)
{
    if (is_int(INPUT(0))) {
        int key = as_int(INPUT(0));
        for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++)  {
            if (KEYS_JUST_PRESSED[index].sym == key) {
                set_bool(OUTPUT, true);
                return;
            }
        }
        set_bool(OUTPUT, false);
        return;
    }

    if (is_string(INPUT(0))) {
        std::string const& key = INPUT(0)->asString();
        if (key.length() != 1)
            return error_occurred(CONTEXT_AND_CALLER, "Expected a string of length 1");

        for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++)  {
            if (key[0] == KEYS_JUST_PRESSED[index].unicode) {
                set_bool(OUTPUT, true);
                return;
            }
        }
            
        set_bool(OUTPUT, false);
        return;
    }
}

void capture_events()
{
    KEYS_JUST_PRESSED.clear();
    RECENT_LEFT_MOUSE_DOWN = false;
    RECENT_MOUSE_WHEEL_UP = false;
    RECENT_MOUSE_WHEEL_DOWN = false;

    // Consume all events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_QUIT:
            app::singleton()._continueMainLoop = false;
            break;

        case SDL_VIDEORESIZE:
            resize_display(event.resize.w, event.resize.h);
            break;

        case SDL_KEYDOWN:

            if (!KEY_DOWN[event.key.keysym.sym]) {
                KEYS_JUST_PRESSED.push_back(event.key.keysym);
                handle_key_press(event, event.key.keysym.sym);
            }

            KEY_DOWN[event.key.keysym.sym] = true;
            break;

        case SDL_KEYUP:
            KEY_DOWN[event.key.keysym.sym] = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                RECENT_LEFT_MOUSE_DOWN = true;
                LEFT_MOUSE_DOWN = true;
            } else if (event.button.button == SDL_BUTTON_WHEELUP)
                RECENT_MOUSE_WHEEL_UP = true;
            else if (event.button.button == SDL_BUTTON_WHEELDOWN)
                RECENT_MOUSE_WHEEL_DOWN = true;
            MOUSE_X = event.button.x;
            MOUSE_Y = event.button.y;
            break;

        case SDL_MOUSEBUTTONUP:
            MOUSE_X = event.button.x;
            MOUSE_Y = event.button.y;
            if (event.button.button == SDL_BUTTON_LEFT)
                LEFT_MOUSE_DOWN = false;
            break;
        case SDL_MOUSEMOTION:
            MOUSE_X = event.motion.x;
            MOUSE_Y = event.motion.y;
            break;
        }
    } // finish event loop

    set_float(MOUSE_POSITION_TERM->getIndex(0), float(MOUSE_X));
    set_float(MOUSE_POSITION_TERM->getIndex(1), float(MOUSE_Y));
}

void handle_key_press(SDL_Event &event, int key)
{
    bool controlPressed = event.key.keysym.mod & KMOD_CTRL;

    // Unmodified keys
    if (!controlPressed) {
        switch (key) {
        case SDLK_ESCAPE:
            app::singleton()._continueMainLoop = false;
            break;
        default: break;
        }
    }

    // Control keys
    if (controlPressed) {
        switch (event.key.keysym.sym) {
        case SDLK_s:
            persist_branch_to_file(app::users_branch());
            std::cout << "saved to " << get_branch_source_filename(app::users_branch()) << std::endl;
            break;

        case SDLK_e:
            reset_state(app::users_branch());
            if (app::paused() && app::pause_reason() == PauseStatus::RUNTIME_ERROR)
                app::unpause();
            break;

        case SDLK_r:
            app::reload_runtime();
            break;

        case SDLK_p:
            if (app::paused()) {
                std::cout << "Unpaused" << std::endl;
                app::unpause();
            } else {
                std::cout << "Paused" << std::endl;
                app::pause(PauseStatus::USER_REQUEST);
            }
            break;

        // toggle low-power mode
        case SDLK_l: {
            if (app::singleton()._targetFps == 6)
                app::singleton()._targetFps = 60;
            else
                app::singleton()._targetFps = 6;
            std::cout << "target FPS = " << app::singleton()._targetFps << std::endl;
            break;
        }

        default: break;
        }
    }
}

CA_FUNCTION(recent_key_presses)
{
    List* output = (List*) OUTPUT;
    output->clear();

    for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++) {
        List* item = (List*) make_list(output->append());
        Uint16 key = KEYS_JUST_PRESSED[index].unicode;
        char keyStr[2];
        keyStr[1] = 0;

        // check to see if the character is unprintable. This check is not that great.
        if ((key & 0xf700) || key == 0x7f)
            keyStr[0] = 0;
        else
            keyStr[0] = char(key);

        make_string(item->append(), keyStr);
        make_int(item->append(), KEYS_JUST_PRESSED[index].sym);
        make_int(item->append(), KEYS_JUST_PRESSED[index].mod);
    }
}

bool mouse_in(TaggedValue* box)
{
    float x1 = box->getIndex(0)->toFloat();
    float y1 = box->getIndex(1)->toFloat();
    float x2 = box->getIndex(2)->toFloat();
    float y2 = box->getIndex(3)->toFloat();
    return x1 <= MOUSE_X && y1 <= MOUSE_Y
        && x2 >= MOUSE_X && y2 >= MOUSE_Y;
}

CA_FUNCTION(mouse_pressed)
{
    set_bool(OUTPUT, LEFT_MOUSE_DOWN);
}

CA_FUNCTION(mouse_clicked)
{
    if (NUM_INPUTS == 0)
        set_bool(OUTPUT, RECENT_LEFT_MOUSE_DOWN);
    else
        set_bool(OUTPUT, RECENT_LEFT_MOUSE_DOWN && mouse_in(INPUT(0)));
}

CA_FUNCTION(mouse_over)
{
    set_bool(OUTPUT, mouse_in(INPUT(0)));
}

CA_FUNCTION(mouse_wheel_up)
{
    if (NUM_INPUTS == 0)
        set_bool(OUTPUT, RECENT_MOUSE_WHEEL_UP);
    else
        set_bool(OUTPUT, RECENT_MOUSE_WHEEL_UP && mouse_in(INPUT(0)));
}

CA_FUNCTION(mouse_wheel_down)
{
    if (NUM_INPUTS == 0)
        set_bool(OUTPUT, RECENT_MOUSE_WHEEL_DOWN);
    else
        set_bool(OUTPUT, RECENT_MOUSE_WHEEL_DOWN && mouse_in(INPUT(0)));
}

void setup(Branch& branch)
{
    for (int i=0; i < SDLK_LAST; i++)
        KEY_DOWN[i] = false;

    MOUSE_POSITION_TERM = branch.findFirstBinding("mouse");

    install_function(branch["key_down"], key_down);
    install_function(branch["key_pressed_code"], key_pressed);
    install_function(branch["key_pressed_char"], key_pressed);
    install_function(branch["recent_key_presses"], recent_key_presses);
    install_function(branch["mouse_pressed"], mouse_pressed);
    install_function(branch["mouse_over"], mouse_over);

    install_function(branch["mouse_clicked_anywhere"], mouse_clicked);
    install_function(branch["mouse_clicked_region"], mouse_clicked);
    install_function(branch["mouse_wheel_up_anywhere"], mouse_wheel_up);
    install_function(branch["mouse_wheel_up_region"], mouse_wheel_up);
    install_function(branch["mouse_wheel_down_anywhere"], mouse_wheel_down);
    install_function(branch["mouse_wheel_down_region"], mouse_wheel_down);

    // Initialize key constants
    Branch& key = branch["key"]->nestedContents;
    assert(is_namespace(key));

    set_int(key["up"], SDLK_UP);
    set_int(key["down"], SDLK_DOWN);
    set_int(key["left"], SDLK_LEFT);
    set_int(key["right"], SDLK_RIGHT);
    set_int(key["space"], SDLK_SPACE);
    set_int(key["delete"], SDLK_BACKSPACE);
    set_int(key["enter"], SDLK_RETURN);
    set_int(key["escape"], SDLK_ESCAPE);
}

} // namespace input
