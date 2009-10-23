// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

#include "plastic.h"

using namespace circa;

namespace input {

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool LEFT_MOUSE_DOWN = false;
bool RECENT_LEFT_MOUSE_DOWN = false;
bool RECENT_MOUSE_WHEEL_UP = false;
bool RECENT_MOUSE_WHEEL_DOWN = false;

Ref MOUSE_POSITION_TERM;

void key_down(Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_DOWN[i];
}

void key_pressed(Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_JUST_PRESSED.find(i) != KEY_JUST_PRESSED.end();
}

void capture_events()
{
    KEY_JUST_PRESSED.clear();
    RECENT_LEFT_MOUSE_DOWN = false;
    RECENT_MOUSE_WHEEL_UP = false;
    RECENT_MOUSE_WHEEL_DOWN = false;

    // Consume all events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            CONTINUE_MAIN_LOOP = false;

        } else if (event.type == SDL_KEYDOWN) {

            if (!KEY_DOWN[event.key.keysym.sym]) {
                KEY_JUST_PRESSED.insert(event.key.keysym.sym);
                handle_key_press(event, event.key.keysym.sym);
            }

            KEY_DOWN[event.key.keysym.sym] = true;

        } else if (event.type == SDL_KEYUP) {
            KEY_DOWN[event.key.keysym.sym] = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                RECENT_LEFT_MOUSE_DOWN = true;
                LEFT_MOUSE_DOWN = true;
            } else if (event.button.button == SDL_BUTTON_WHEELUP)
                RECENT_MOUSE_WHEEL_UP = true;
            else if (event.button.button == SDL_BUTTON_WHEELDOWN)
                RECENT_MOUSE_WHEEL_DOWN = true;
            MOUSE_X = event.button.x;
            MOUSE_Y = event.button.y;

        } else if (event.type == SDL_MOUSEBUTTONUP) {
            MOUSE_X = event.button.x;
            MOUSE_Y = event.button.y;
            if (event.button.button == SDL_BUTTON_LEFT)
                LEFT_MOUSE_DOWN = false;
        } else if (event.type == SDL_MOUSEMOTION) {
            MOUSE_X = event.motion.x;
            MOUSE_Y = event.motion.y;
        }
    } // finish event loop

    MOUSE_POSITION_TERM->asBranch()[0]->asFloat() = float(MOUSE_X);
    MOUSE_POSITION_TERM->asBranch()[1]->asFloat() = float(MOUSE_Y);
}

void handle_key_press(SDL_Event &event, int key)
{
    // Unmodified keys
    switch (key) {
    case SDLK_ESCAPE:
        CONTINUE_MAIN_LOOP = false;
        break;

    case SDLK_e:
        reset_state(*USERS_BRANCH);
        if (PAUSED && PAUSE_REASON == RUNTIME_ERROR)
            PAUSED = false;
        break;

    case SDLK_r:
        reload_branch_from_file(*USERS_BRANCH, std::cout);
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
            persist_branch_to_file(*USERS_BRANCH);
            std::cout << "saved to " << get_branch_source_filename(*USERS_BRANCH) << std::endl;
            break;

        case SDLK_p:
            std::cout << print_branch_raw(*USERS_BRANCH);
            break;

        default: break;
        }
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
    as_bool(caller) = LEFT_MOUSE_DOWN;
}

void mouse_clicked(Term* caller)
{
    if (caller->numInputs() == 0)
        as_bool(caller) = RECENT_LEFT_MOUSE_DOWN;
    else
        as_bool(caller) = RECENT_LEFT_MOUSE_DOWN && mouse_in(as_branch(caller->input(0)));
}

void mouse_over(Term* caller)
{
    as_bool(caller) = mouse_in(as_branch(caller->input(0)));
}

void mouse_wheel_up(Term* caller)
{
    if (caller->numInputs() == 0)
        as_bool(caller) = RECENT_MOUSE_WHEEL_UP;
    else
        as_bool(caller) = RECENT_MOUSE_WHEEL_UP && mouse_in(as_branch(caller->input(0)));
}

void mouse_wheel_down(Term* caller)
{
    if (caller->numInputs() == 0)
        as_bool(caller) = RECENT_MOUSE_WHEEL_DOWN;
    else
        as_bool(caller) = RECENT_MOUSE_WHEEL_DOWN && mouse_in(as_branch(caller->input(0)));
}

void initialize(Branch& branch)
{
    int_value(branch, SDLK_UP, "UP");
    int_value(branch, SDLK_DOWN, "DOWN");
    int_value(branch, SDLK_LEFT, "LEFT");
    int_value(branch, SDLK_RIGHT, "RIGHT");
    int_value(branch, SDLK_SPACE, "SPACE");
    int_value(branch, SDLK_b, "KEY_B");
}

void setup(Branch& branch)
{
    for (int i=0; i < SDLK_LAST; i++)
        KEY_DOWN[i] = false;

    MOUSE_POSITION_TERM = branch.findFirstBinding("mouse");

    install_function(branch["key_down"], key_down);
    install_function(branch["key_pressed"], key_pressed);
    install_function(branch["mouse_pressed"], mouse_pressed);
    install_function(branch["mouse_over"], mouse_over);

    install_function(branch["mouse_clicked"]->asBranch()[0], mouse_clicked);
    install_function(branch["mouse_clicked"]->asBranch()[1], mouse_clicked);
    install_function(branch["mouse_wheel_up"]->asBranch()[0], mouse_wheel_up);
    install_function(branch["mouse_wheel_up"]->asBranch()[1], mouse_wheel_up);
    install_function(branch["mouse_wheel_down"]->asBranch()[0], mouse_wheel_down);
    install_function(branch["mouse_wheel_down"]->asBranch()[1], mouse_wheel_down);
}

} // namespace input
