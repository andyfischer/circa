// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include "plastic.h"

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

void key_down(Term* caller)
{
    int i = caller->input(0)->asInt();
    set_bool(caller, KEY_DOWN[i]);
}

void key_pressed(Term* caller)
{
    if (is_int(caller->input(0))) {
        int key = as_int(caller->input(0));
        for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++)  {
            if (KEYS_JUST_PRESSED[index].sym == key) {
                set_bool(caller, true);
                return;
            }
        }
        set_bool(caller, false);
        return;
    }

    if (is_string(caller->input(0))) {
        std::string const& key = caller->input(0)->asString();
        if (key.length() != 1) {
            error_occurred(caller, "Expected a string of length 1");
            return;
        }

        for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++)  {
            if (key[0] == KEYS_JUST_PRESSED[index].unicode) {
                set_bool(caller, true);
                return;
            }
        }
            
        set_bool(caller, false);
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
            CONTINUE_MAIN_LOOP = false;
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

    set_float(MOUSE_POSITION_TERM->asBranch()[0], float(MOUSE_X));
    set_float(MOUSE_POSITION_TERM->asBranch()[1], float(MOUSE_Y));
}

void handle_key_press(SDL_Event &event, int key)
{
    // Unmodified keys
    switch (key) {
    case SDLK_ESCAPE:
        CONTINUE_MAIN_LOOP = false;
        break;

    case SDLK_e:
        reset_state(users_branch());
        if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
            PAUSED = false;
        break;

    case SDLK_r:
        reload_branch_from_file(users_branch(), std::cout);
        if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
            PAUSED = false;
        break;

    case SDLK_p:
        if (PAUSED) {
            std::cout << "Unpaused" << std::endl;
            PAUSED = false;
        } else {
            std::cout << "Paused" << std::endl;
            PAUSED = true;
            PAUSE_REASON = USER_REQUEST;
        }
        break;

    // toggle low-power mode
    case SDLK_l:
        if (TARGET_FPS == 6)
            TARGET_FPS = 60;
        else
            TARGET_FPS = 6;
        std::cout << "target FPS = " << TARGET_FPS << std::endl;
        break;
    }

    // Control keys
    if (event.key.keysym.mod & KMOD_CTRL) {
        switch (event.key.keysym.sym) {
        case SDLK_s:
            persist_branch_to_file(users_branch());
            std::cout << "saved to " << get_branch_source_filename(users_branch()) << std::endl;
            break;

        case SDLK_p:
            print_branch_raw(std::cout, users_branch());
            break;

        default: break;
        }
    }
}

void recent_key_presses(Term* caller)
{
    Branch& output = as_branch(caller);
    output.clear();

    for (size_t index=0; index < KEYS_JUST_PRESSED.size(); index++) {
        Branch& item = create_list(output);
        Uint16 key = KEYS_JUST_PRESSED[index].unicode;
        char keyStr[2];
        keyStr[1] = 0;

        // check to see if the character is unprintable. This check is not that great.
        if ((key & 0xf700) || key == 0x7f)
            keyStr[0] = 0;
        else
            keyStr[0] = char(key);

        // when pressing some special characters, SDL gives us some low numbers
        // for the characters (such as 1 for up). Not sure of the 

        create_string(item, std::string(keyStr));
        create_int(item, KEYS_JUST_PRESSED[index].sym);
    }
}

bool mouse_in(Branch& box)
{
    float x1 = box[0]->toFloat();
    float y1 = box[1]->toFloat();
    float x2 = box[2]->toFloat();
    float y2 = box[3]->toFloat();
    return x1 <= MOUSE_X && y1 <= MOUSE_Y
        && x2 >= MOUSE_X && y2 >= MOUSE_Y;
}

void mouse_pressed(Term* caller)
{
    set_bool(caller, LEFT_MOUSE_DOWN);
}

void mouse_clicked(Term* caller)
{
    if (caller->numInputs() == 0)
        set_bool(caller, RECENT_LEFT_MOUSE_DOWN);
    else
        set_bool(caller, RECENT_LEFT_MOUSE_DOWN && mouse_in(as_branch(caller->input(0))));
}

void mouse_over(Term* caller)
{
    set_bool(caller, mouse_in(as_branch(caller->input(0))));
}

void mouse_wheel_up(Term* caller)
{
    if (caller->numInputs() == 0)
        set_bool(caller, RECENT_MOUSE_WHEEL_UP);
    else
        set_bool(caller, RECENT_MOUSE_WHEEL_UP && mouse_in(as_branch(caller->input(0))));
}

void mouse_wheel_down(Term* caller)
{
    if (caller->numInputs() == 0)
        set_bool(caller, RECENT_MOUSE_WHEEL_DOWN);
    else
        set_bool(caller, RECENT_MOUSE_WHEEL_DOWN && mouse_in(as_branch(caller->input(0))));
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

    set_int(procure_int(branch, "UP"), SDLK_UP);
    set_int(procure_int(branch, "DOWN"), SDLK_DOWN);
    set_int(procure_int(branch, "LEFT"), SDLK_LEFT);
    set_int(procure_int(branch, "RIGHT"), SDLK_RIGHT);
    set_int(procure_int(branch, "SPACE"), SDLK_SPACE);
    set_int(procure_int(branch, "KEY_B"), SDLK_b);
    set_int(procure_int(branch, "DELETE"), SDLK_DELETE);
}

} // namespace input
