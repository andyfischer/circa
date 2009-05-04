// Copyright 2008 Paul Hodge

#include "SDL.h"

#include <string>

#include "circa.h"

using namespace circa;

#include "sdl_wrapper.cpp"

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

bool KEY_DOWN[SDLK_LAST];
std::set<int> KEY_JUST_PRESSED;

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool MOUSE_JUST_CLICKED = false;
int MOUSE_CLICK_FOUND_ID = 0;
int MOUSE_CLICK_ACCEPTED_ID = 0;

SDL_Surface* SCREEN = NULL;
circa::Branch SCRIPT_MAIN;
bool CONTINUE_MAIN_LOOP = true;

circa::RefList INFLUENCE_LIST;

bool drag_in_progress = false;

int previous_MOUSE_X = 0;
int previous_MOUSE_Y = 0;

int drag_start_x = 0;
int drag_start_y = 0;

long prev_sdl_ticks = 0;

circa::Ref THING_JUST_CLICKED;

circa::Ref MOUSE_CLICKED_FUNCTION;

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

void mouse_clicked(circa::Term* caller)
{
    static int unassignedId = 1;

    int id = as_int(caller->state);
    //std::cout << "id = " << id << std::endl;
    
    // check to give this term an ID
    if (id == 0)
        as_int(caller->state) = unassignedId++;

    // check if we just clicked this thing
    Term* box = caller->input(0);
    if (MOUSE_JUST_CLICKED &&
        (as_branch(box)[0]->asFloat() < MOUSE_X) &&
        (as_branch(box)[1]->asFloat() < MOUSE_Y) &&
        (as_branch(box)[2]->asFloat() > MOUSE_X) &&
        (as_branch(box)[3]->asFloat() > MOUSE_Y)) {
        MOUSE_CLICK_FOUND_ID = id;
    }
    
    as_bool(caller) = id == MOUSE_CLICK_ACCEPTED_ID;

    //std::cout << "MOUSE_CLICK_ACCEPTED_ID = " << MOUSE_CLICK_ACCEPTED_ID << std::endl;
    //std::cout << "MOUSE_CLICK_FOUND_ID = " << MOUSE_CLICK_FOUND_ID << std::endl;
}

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

/*
Term* find_mouse_clicked(Branch& branch, int mouse_x, int mouse_y)
{
    Term* result = NULL;

    // todo: use an iterator that only descends down active branches
    for (circa::CodeIterator it(&branch); !it.finished(); ++it) {
        if (it->function == MOUSE_CLICKED_FUNCTION) {
            Term* box = it->input(0);
            if ((as_branch(box)[0]->asFloat() < mouse_x) &&
                (as_branch(box)[1]->asFloat() < mouse_y) &&
                (as_branch(box)[2]->asFloat() > mouse_y) &&
                (as_branch(box)[3]->asFloat() > mouse_y)) {
                result = *it;
            }
        }
    }

    return result;
}
*/

int main( int argc, char* args[] )
{
    // Initialize stuff
    initialize_keydown();

    circa::initialize();

    // Import functions
    circa::import_function(*circa::KERNEL, key_down, "key_down(int) : bool");
    circa::import_function(*circa::KERNEL, key_pressed, "key_pressed(int) : bool");
    circa::int_value(circa::KERNEL, SDLK_UP, "KEY_UP");
    circa::int_value(circa::KERNEL, SDLK_DOWN, "KEY_DOWN");
    circa::int_value(circa::KERNEL, SDLK_LEFT, "KEY_LEFT");
    circa::int_value(circa::KERNEL, SDLK_RIGHT, "KEY_RIGHT");
    circa::int_value(circa::KERNEL, SDLK_SPACE, "KEY_SPACE");
    MOUSE_CLICKED_FUNCTION = circa::import_function(
        *circa::KERNEL, mouse_clicked, "mouse_clicked(List) : bool");
    circa::as_function(MOUSE_CLICKED_FUNCTION).stateType = circa::INT_TYPE;
    sdl_wrapper::register_functions(*circa::KERNEL);

    // Load the target script
    std::string filename;
    if (argc > 1) filename = args[1];
    else filename = "proto7/main.ca";

    std::cout << "Loading file: " << filename << std::endl;

    circa::evaluate_file(SCRIPT_MAIN, filename);
    refresh_training_branch(SCRIPT_MAIN);

    // See if this script defined SCREEN_WIDTH or SCREEN_HEIGHT
    if (SCRIPT_MAIN.contains("SCREEN_WIDTH"))
        SCREEN_WIDTH = circa::to_float(SCRIPT_MAIN["SCREEN_WIDTH"]);
    if (SCRIPT_MAIN.contains("SCREEN_HEIGHT"))
        SCREEN_HEIGHT = circa::to_float(SCRIPT_MAIN["SCREEN_HEIGHT"]);

    // Create the SDL surface
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    // If there was an error in setting up the screen
    if (SCREEN == NULL)
        return 1;

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1)
        return 1;

    // Set the window caption
    SDL_WM_SetCaption("proto7", NULL);

    prev_sdl_ticks = SDL_GetTicks();

    // Main loop
    while (CONTINUE_MAIN_LOOP) {
        KEY_JUST_PRESSED.clear();
        MOUSE_JUST_CLICKED = false;
        MOUSE_CLICK_ACCEPTED_ID = MOUSE_CLICK_FOUND_ID;
        MOUSE_CLICK_FOUND_ID = 0;

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
                drag_start_x = MOUSE_X;
                drag_start_y = MOUSE_Y;

                MOUSE_JUST_CLICKED = true;

                //THING_JUST_CLICKED = find_mouse_clicked(SCRIPT_MAIN, MOUSE_X, MOUSE_Y);

            } else if (event.type == SDL_MOUSEBUTTONUP) {
                // draw a line
                if (drag_in_progress) {
                    std::cout << "drawing a line" << std::endl;
                    circa::apply(&SCRIPT_MAIN, "line", circa::RefList(
                                circa::float_value(&SCRIPT_MAIN, drag_start_x),
                                circa::float_value(&SCRIPT_MAIN, drag_start_y),
                                circa::float_value(&SCRIPT_MAIN, MOUSE_X),
                                circa::float_value(&SCRIPT_MAIN, MOUSE_Y),
                                circa::int_value(&SCRIPT_MAIN, 0)));
                }

                drag_in_progress = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                MOUSE_X = event.motion.x;
                MOUSE_Y = event.motion.y;
            }
        } // finish event loop

        long ticks = SDL_GetTicks();

        circa::as_float(SCRIPT_MAIN["elapsed"]) = (ticks - prev_sdl_ticks) / 1000.0;
        circa::as_float(SCRIPT_MAIN["time"]) = ticks / 1000.0;

        prev_sdl_ticks = ticks;

        circa::as_float(SCRIPT_MAIN["mouse_x"]) = MOUSE_X;
        circa::as_float(SCRIPT_MAIN["mouse_y"]) = MOUSE_Y;

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
