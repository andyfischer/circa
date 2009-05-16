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
Branch* SCRIPT_ROOT = NULL;
Branch* USERS_BRANCH = NULL;
bool CONTINUE_MAIN_LOOP = true;

circa::RefList INFLUENCE_LIST;

bool drag_in_progress = false;

int previous_MOUSE_X = 0;
int previous_MOUSE_Y = 0;

int drag_start_x = 0;
int drag_start_y = 0;

float TIME = 0;
float ELAPSED = 0;
long prev_sdl_ticks = 0;

Ref THING_JUST_CLICKED;
Ref MOUSE_CLICKED_FUNCTION;

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

    int& id = get_hidden_state_for_call(caller)->asInt();
    
    // check to give this term an ID
    if (id == 0)
        id = unassignedId++;

    // check if we just clicked this thing
    Term* box = caller->input(1);
    if (MOUSE_JUST_CLICKED &&
        (as_branch(box)[0]->asFloat() < MOUSE_X) &&
        (as_branch(box)[1]->asFloat() < MOUSE_Y) &&
        (as_branch(box)[2]->asFloat() > MOUSE_X) &&
        (as_branch(box)[3]->asFloat() > MOUSE_Y)) {
        MOUSE_CLICK_FOUND_ID = id;
    }
    
    as_bool(caller) = id == MOUSE_CLICK_ACCEPTED_ID;
}

void mouse_pressed(circa::Term* caller)
{
    as_bool(caller) = MOUSE_JUST_CLICKED;
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
            circa::persist_branch_to_file(*USERS_BRANCH);
            std::cout << "Saved" << std::endl;
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

int main( int argc, char* args[] )
{
    // Initialize stuff
    initialize_keydown();

    circa::initialize();

    SCRIPT_ROOT = &create_branch(circa::KERNEL, "c2d_root");

    // Import constants
    expose_value(SCRIPT_ROOT, &MOUSE_X, "mouse_x");
    expose_value(SCRIPT_ROOT, &MOUSE_Y, "mouse_y");
    expose_value(SCRIPT_ROOT, &TIME, "time");
    expose_value(SCRIPT_ROOT, &ELAPSED, "elapsed");

    // Import functions
    circa::import_function(*SCRIPT_ROOT, key_down, "key_down(int) : bool");
    circa::import_function(*SCRIPT_ROOT, key_pressed, "key_pressed(int) : bool");
    circa::int_value(SCRIPT_ROOT, SDLK_UP, "KEY_UP");
    circa::int_value(SCRIPT_ROOT, SDLK_DOWN, "KEY_DOWN");
    circa::int_value(SCRIPT_ROOT, SDLK_LEFT, "KEY_LEFT");
    circa::int_value(SCRIPT_ROOT, SDLK_RIGHT, "KEY_RIGHT");
    circa::int_value(SCRIPT_ROOT, SDLK_SPACE, "KEY_SPACE");
    MOUSE_CLICKED_FUNCTION = circa::import_function(
        *SCRIPT_ROOT, mouse_clicked, "mouse_clicked(state int, List) : bool");
    circa::import_function(*circa::KERNEL, mouse_pressed, "mouse_pressed() : bool");

    sdl_wrapper::register_functions(*SCRIPT_ROOT);

    // Load runtime.ca
    parse_script(*SCRIPT_ROOT, "cuttlefish/runtime.ca");

    // Load user's script
    USERS_BRANCH = &SCRIPT_ROOT->get("users_branch")->asBranch();

    if (argc > 1) {
        std::string filename = args[1];
        std::cout << "Loading file: " << filename << std::endl;
        circa::parse_script(*USERS_BRANCH, filename);
    }

    // Check if they defined SCREEN_WIDTH or SCREEN_HEIGHT
    if (USERS_BRANCH->contains("SCREEN_WIDTH"))
        SCREEN_WIDTH = USERS_BRANCH->get("SCREEN_WIDTH")->toFloat();
    if (USERS_BRANCH->contains("SCREEN_HEIGHT"))
        SCREEN_HEIGHT = USERS_BRANCH->get("SCREEN_HEIGHT")->toFloat();

    // Create the SDL surface
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    // If there was an error in setting up the screen
    if (SCREEN == NULL)
        return 1;

    // Initialize desired SDL subsystems
    if (SDL_Init(SDL_INIT_TIMER & SDL_INIT_VIDEO & SDL_INIT_JOYSTICK & SDL_INIT_EVENTTHREAD) == -1)
        return 1;

    // Set the window caption
    SDL_WM_SetCaption("Cuttlefish game engine", NULL);

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

            } else if (event.type == SDL_MOUSEBUTTONUP) {
                // draw a line
                if (drag_in_progress) {
                    std::cout << "drawing a line" << std::endl;
                    circa::apply(USERS_BRANCH, "line", circa::RefList(
                                circa::float_value(USERS_BRANCH, drag_start_x),
                                circa::float_value(USERS_BRANCH, drag_start_y),
                                circa::float_value(USERS_BRANCH, MOUSE_X),
                                circa::float_value(USERS_BRANCH, MOUSE_Y),
                                circa::int_value(USERS_BRANCH, 0)));
                }

                drag_in_progress = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                MOUSE_X = event.motion.x;
                MOUSE_Y = event.motion.y;
            }
        } // finish event loop

        long ticks = SDL_GetTicks();

        ELAPSED = (ticks - prev_sdl_ticks) / 1000.0;
        TIME = ticks / 1000.0;

        prev_sdl_ticks = ticks;

        try {
            SCRIPT_ROOT->eval();

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
