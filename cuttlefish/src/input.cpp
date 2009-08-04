// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "SDL.h"

#include "input.h"
#include "main.h"

using namespace circa;

namespace input {

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

Int MOUSE_X;
Int MOUSE_Y;
bool LEFT_MOUSE_DOWN = false;
bool RECENT_LEFT_MOUSE_DOWN;
bool RECENT_MOUSE_WHEEL_UP;
bool RECENT_MOUSE_WHEEL_DOWN;

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
}

void handle_key_press(SDL_Event &event, int key)
{
    // Unmodified keys
    switch (key) {
        case SDLK_ESCAPE:
            CONTINUE_MAIN_LOOP = false;

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
                PAUSED = false;
            } else {
                PAUSED = true;
                PAUSE_REASON = USER_REQUEST;
            }
            break;
    }

    // Control keys
    if (event.key.keysym.mod & KMOD_CTRL) {
        switch (event.key.keysym.sym) {
        case SDLK_s:
            circa::persist_branch_to_file(*USERS_BRANCH);
            std::cout << "Saved" << std::endl;
            break;

        case SDLK_p:
            std::cout << branch_to_string_raw(*USERS_BRANCH);
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
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
    }

    MOUSE_X = int_value(branch, 0, "_mouse_x");
    MOUSE_Y = int_value(branch, 0, "_mouse_y");
    import_function(branch, key_down, "key_down(int) : bool");
    import_function(branch, key_pressed, "key_pressed(int) : bool");
    import_function(branch, mouse_pressed, "mouse_pressed() : bool");
    int_value(branch, SDLK_UP, "UP");
    int_value(branch, SDLK_DOWN, "DOWN");
    int_value(branch, SDLK_LEFT, "LEFT");
    int_value(branch, SDLK_RIGHT, "RIGHT");
    int_value(branch, SDLK_SPACE, "SPACE");

    import_function(branch, mouse_over, "mouse_over(List region) : bool");

    Term* mouse_clicked_func = create_overloaded_function(branch, "mouse_clicked");
    import_function_overload(mouse_clicked_func, mouse_clicked, "mouse_clicked() : bool");
    import_function_overload(mouse_clicked_func, mouse_clicked, "mouse_clicked(List region) : bool");

    Term* mouse_wheel_up_func = create_overloaded_function(branch, "mouse_wheel_up");
    import_function_overload(mouse_wheel_up_func, mouse_wheel_up, "mouse_wheel_up() : bool");
    import_function_overload(mouse_wheel_up_func, mouse_wheel_up, "mouse_wheel_up(List region) : bool");

    Term* mouse_wheel_down_func = create_overloaded_function(branch, "mouse_wheel_down");
    import_function_overload(mouse_wheel_down_func, mouse_wheel_down, "mouse_wheel_down(List region) : bool");
    import_function_overload(mouse_wheel_down_func, mouse_wheel_down, "mouse_wheel_down() : bool");
}

} // namespace input
