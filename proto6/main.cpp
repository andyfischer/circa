// Copyright 2008 Andrew Fischer

#include "SDL.h"
#include "SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

bool mouse_just_pressed = false;
int mouse_x = 0;
int mouse_y = 0;

SDL_Surface* SCREEN = NULL;
circa::Branch SCRIPT_MAIN;
bool CONTINUE_MAIN_LOOP = true;

circa::RefList INFLUENCE_LIST;

bool drag_in_progress = false;

int previous_mouse_x = 0;
int previous_mouse_y = 0;

int drag_start_x = 0;
int drag_start_y = 0;

void initialize_keydown()
{
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
    }
}

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

namespace sdl_hosted {

void box(circa::Term* caller)
{
    boxColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        (unsigned int) as_int(caller->input(4)));
}

void line(circa::Term* caller)
{
    aalineColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

void background(circa::Term* caller)
{
    boxColor(SCREEN, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
            as_int(caller->input(0)));
}

void shape(circa::Term* caller)
{
    static Sint16 vx[500];
    static Sint16 vy[500];

    circa::Branch& list = circa::as_branch(caller->input(0));
    for (int i=0; i < list.numTerms(); i++) {
        vx[i] = circa::to_float(list[i]->field(0));
        vy[i] = circa::to_float(list[i]->field(1));
    }
    filledPolygonColor(SCREEN, vx, vy, list.numTerms(), caller->input(1)->asInt());
}

void drawText(circa::Term* caller)
{
    stringColor(SCREEN, caller->input(0)->asFloat(), caller->input(1)->asFloat(),
            caller->input(2)->asString().c_str(), caller->input(3)->asInt());
}

void mouse_pressed(circa::Term* caller)
{
    circa::as_bool(caller) = mouse_just_pressed;
}

} // namespace sdl_hosted

void handle_key_press(SDL_Event event, int key)
{
    // unmodified keys
    switch (key) {

    case SDLK_ESCAPE: CONTINUE_MAIN_LOOP = false; break;
    default: break;

    }

    // Control keys
    if (event.key.keysym.mod & KMOD_CTRL) {
        switch(event.key.keysym.sym) {
        case SDLK_s:
            circa::persist_branch_to_file(SCRIPT_MAIN);
            std::cout << "Saved" << std::endl;
            break;

        case SDLK_p:
            std::cout << branch_to_string_raw(SCRIPT_MAIN);
            break;

        case SDLK_r:
            circa::reload_branch_from_file(SCRIPT_MAIN);
            refresh_training_branch(SCRIPT_MAIN);
            break;

        default: break;
        }
    }
}

int main( int argc, char* args[] )
{
    initialize_keydown();

    circa::initialize();

    circa::import_function(*circa::KERNEL, key_down, "key_down(int) -> bool");
    circa::import_function(*circa::KERNEL, key_pressed, "key_pressed(int) -> bool");
    circa::int_value(circa::KERNEL, SDLK_UP, "KEY_UP");
    circa::int_value(circa::KERNEL, SDLK_DOWN, "KEY_DOWN");
    circa::int_value(circa::KERNEL, SDLK_LEFT, "KEY_LEFT");
    circa::int_value(circa::KERNEL, SDLK_RIGHT, "KEY_RIGHT");
    circa::int_value(circa::KERNEL, SDLK_SPACE, "KEY_SPACE");

    circa::import_function(*circa::KERNEL, sdl_hosted::box, "box(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, sdl_hosted::line, "line(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, sdl_hosted::background, "background(int)");
    circa::import_function(*circa::KERNEL, sdl_hosted::shape, "shape(List,int)");
    circa::import_function(*circa::KERNEL, sdl_hosted::drawText, "drawText(float,float, string, int)");
    circa::import_function(*circa::KERNEL, sdl_hosted::mouse_pressed, "mouse_pressed() : bool");

    // Set up the screen
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    // If there was an error in setting up the screen
    if( SCREEN == NULL )
        return 1;

    std::string filename;
    if (argc > 1)
        filename = args[1];
    else
        filename = "proto6/main.ca";

    std::cout << "loading file: " << filename << std::endl;

    circa::evaluate_file(SCRIPT_MAIN, filename);
    refresh_training_branch(SCRIPT_MAIN);

    // Initialize all SDL subsystems
    if( SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1)
        return 1;

    // Set the window caption
    SDL_WM_SetCaption("proto6", NULL);

    // Main loop
    while (CONTINUE_MAIN_LOOP) {
        KEY_JUST_PRESSED.clear();

        mouse_just_pressed = false;

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
                mouse_just_pressed = true;
                drag_start_x = mouse_x;
                drag_start_y = mouse_y;
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                // draw a line
                if (drag_in_progress) {
                    std::cout << "drawing a line" << std::endl;
                    circa::apply(&SCRIPT_MAIN, "line", circa::RefList(
                                circa::float_value(&SCRIPT_MAIN, drag_start_x),
                                circa::float_value(&SCRIPT_MAIN, drag_start_y),
                                circa::float_value(&SCRIPT_MAIN, mouse_x),
                                circa::float_value(&SCRIPT_MAIN, mouse_y),
                                circa::int_value(&SCRIPT_MAIN, 0)));
                }

                drag_in_progress = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
            }
        } // finish event loop

        circa::as_float(SCRIPT_MAIN["time"]) = SDL_GetTicks() / 1000.0;
        circa::as_float(SCRIPT_MAIN["mouse_x"]) = mouse_x;
        circa::as_float(SCRIPT_MAIN["mouse_y"]) = mouse_y;

        try {
            SCRIPT_MAIN.eval();

        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            return 0;
        }

        // Update the screen
        if( SDL_Flip(SCREEN) == -1 )
            return 1;

        SDL_Delay(10);
    }

    // Quit SDL
    SDL_Quit();

    return 0;
}
