// Copyright 2008 Andrew Fischer

#include "SDL.h"
#include "circa.h"

#include "input.h"
#include "main.h"

using namespace circa;

namespace input {

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool MOUSE_JUST_CLICKED = false;

void key_down(circa::Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_DOWN[i];
}

void key_pressed(circa::Term* caller)
{
    int i = caller->input(0)->asInt();
    caller->asBool() = KEY_JUST_PRESSED.find(i) != KEY_JUST_PRESSED.end();
}

void mouse_pressed(circa::Term* caller)
{
    as_bool(caller) = MOUSE_JUST_CLICKED;
}

void capture_events()
{
    KEY_JUST_PRESSED.clear();
    MOUSE_JUST_CLICKED = false;

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

            MOUSE_JUST_CLICKED = true;

        } else if (event.type == SDL_MOUSEBUTTONUP) {
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
    }

    // Control keys
    if (event.key.keysym.mod & KMOD_CTRL) {
        switch (event.key.keysym.sym) {
        case SDLK_s:
            circa::persist_branch_to_file(*USERS_BRANCH);
            std::cout << "Saved" << std::endl;
            break;

        case SDLK_e:
            circa::reset_state(*USERS_BRANCH);
            std::cout << "State has been reset" << std::endl;
            break;

        case SDLK_p:
            std::cout << branch_to_string_raw(*USERS_BRANCH);
            break;

        case SDLK_r:
            circa::reload_branch_from_file(*USERS_BRANCH);
            break;

        default: break;
        }
    }
}

void region_clicked(Term* caller)
{
    //int& id = caller->input(0)->asInt();
    Branch& region = caller->input(0)->asBranch();
    float x1 = region[0]->toFloat();
    float y1 = region[1]->toFloat();
    float x2 = region[2]->toFloat();
    float y2 = region[3]->toFloat();

#if 0
    if (id == 0) {
        static next_id = 1;
        id = next_id++;
    }
#endif

    as_bool(caller) = (MOUSE_JUST_CLICKED
        && x1 <= MOUSE_X
        && y1 <= MOUSE_Y
        && x2 >= MOUSE_X
        && y2 >= MOUSE_Y);
}

void initialize(Branch& branch)
{
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
    }

    expose_value(&branch, &MOUSE_X, "mouse_x");
    expose_value(&branch, &MOUSE_Y, "mouse_y");
    import_function(branch, key_down, "key_down(int) : bool");
    import_function(branch, key_pressed, "key_pressed(int) : bool");
    import_function(branch, mouse_pressed, "mouse_pressed() : bool");
    int_value(&branch, SDLK_UP, "KEY_UP");
    int_value(&branch, SDLK_DOWN, "KEY_DOWN");
    int_value(&branch, SDLK_LEFT, "KEY_LEFT");
    int_value(&branch, SDLK_RIGHT, "KEY_RIGHT");
    int_value(&branch, SDLK_SPACE, "KEY_SPACE");

    import_function(branch, region_clicked, "region_clicked(List region) : bool");
}

} // namespace input
