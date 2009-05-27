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

void initialize(Branch& branch)
{
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
    }

    circa::expose_value(&branch, &MOUSE_X, "mouse_x");
    circa::expose_value(&branch, &MOUSE_Y, "mouse_y");
    circa::import_function(branch, key_down, "key_down(int) : bool");
    circa::import_function(branch, key_pressed, "key_pressed(int) : bool");
    circa::import_function(branch, mouse_pressed, "mouse_pressed() : bool");
    circa::int_value(&branch, SDLK_UP, "KEY_UP");
    circa::int_value(&branch, SDLK_DOWN, "KEY_DOWN");
    circa::int_value(&branch, SDLK_LEFT, "KEY_LEFT");
    circa::int_value(&branch, SDLK_RIGHT, "KEY_RIGHT");
    circa::int_value(&branch, SDLK_SPACE, "KEY_SPACE");
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
    switch (key) {
        case SDLK_ESCAPE:
            CONTINUE_MAIN_LOOP = false;
    }
}

} // namespace input
