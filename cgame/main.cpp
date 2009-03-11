#include "SDL/SDL.h"
#include "SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
const float HIGHLIGHT_MIN_DIST = 5.0;

bool KEY_DOWN[SDLK_LAST];
bool RECENTLY_PRESSED_KEY[SDLK_LAST];

int MOUSE_X = 0;
int MOUSE_Y = 0;
bool INFLUENCE_LIST_ENABLED = false;

SDL_Surface* SCREEN = NULL;
circa::Term* POINT_FUNC = NULL;
circa::Term* HIGHLIGHT = NULL;
circa::Branch SCRIPT_MAIN;

void initialize_keydown()
{
    for (int i=0; i < SDLK_LAST; i++) {
        KEY_DOWN[i] = false;
        RECENTLY_PRESSED_KEY[i] = false;
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
    caller->asBool() = RECENTLY_PRESSED_KEY[i];
}

void rectangle(circa::Term* caller)
{
    rectangleColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

void fill_rectangle(circa::Term* caller)
{
    boxColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        (unsigned int) as_int(caller->input(4)));
}

void line_to(circa::Term* caller)
{
    aalineColor(SCREEN,
        (int) as_float(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        as_int(caller->input(4)));
}

void update_highlight()
{
    float closestDist = 0;
    HIGHLIGHT = NULL;

    for (circa::CodeIterator it(&SCRIPT_MAIN); !it.finished(); it.advance()) {
        if (it->function == POINT_FUNC) {
            float x = as_float(it->input(0));
            float y = as_float(it->input(1));
            float dist_x = MOUSE_X - x;
            float dist_y = MOUSE_Y - y;
            float dist = sqrt(dist_x*dist_x + dist_y*dist_y);

            if (dist > HIGHLIGHT_MIN_DIST)
                continue;

            if ((dist < closestDist) || HIGHLIGHT == NULL) {
                HIGHLIGHT = *it;
                closestDist = dist;
            }
        }
    }
}

void draw_highlight()
{
    if (HIGHLIGHT == NULL)
        return;

    float x = as_float(HIGHLIGHT->input(0));
    float y = as_float(HIGHLIGHT->input(1));

    circleColor(SCREEN, x, y, HIGHLIGHT_MIN_DIST, 0);
}

void draw_influencers()
{
    if (INFLUENCE_LIST_ENABLED && (HIGHLIGHT != NULL)) {
        if (HIGHLIGHT != NULL) {

            circa::RefList influencers = circa::get_influencing_values(HIGHLIGHT);

            for (int i=0; i < influencers.count(); i++) {
                if (influencers[i]->name == "")
                    influencers[i] = NULL;
            }

            influencers.removeNulls();

            circa::sort_by_name(influencers);

            // background rect
            int rect_width = 8 * 20;
            int rect_height = 9 * influencers.count();
            int rect_top = SCREEN_HEIGHT - rect_height;
            boxColor(SCREEN, 0, rect_top, rect_width, SCREEN_HEIGHT, 0xfffffd);

            for (int i=0; i < influencers.count(); i++) {
                int y = rect_top + i*9;
                stringColor(SCREEN, 0, y, influencers[i]->name.c_str(), 0xff);
            }
        }
    }
}

int main( int argc, char* args[] )
{
    initialize_keydown();

    circa::initialize();

    circa::import_function(*circa::KERNEL, line_to,
            "line_to(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, rectangle,
            "rectangle(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, fill_rectangle,
            "fill_rectangle(float,float,float,float,int)");

    POINT_FUNC = circa::create_empty_function(*circa::KERNEL, "point(float,float)");

    // Set up the screen
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT,
            SCREEN_BPP, SDL_SWSURFACE );

    // If there was an error in setting up the screen
    if( SCREEN == NULL )
        return 1;

    std::string filename;
    if (argc > 1)
        filename = args[1];
    else
        filename = "cgame/main.ca";

    std::cout << "loading file: " << filename << std::endl;

    circa::evaluate_file(SCRIPT_MAIN, filename);

    // Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1)
        return 1;

    // Set the window caption
    SDL_WM_SetCaption( "CGame", NULL );

    // Main loop
    bool continueMainLoop = true;
    while (continueMainLoop) {
        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
            continueMainLoop = false;

        if (event.type == SDL_MOUSEMOTION) {
            SCRIPT_MAIN["mouse_x"]->asFloat() = event.motion.x;
            SCRIPT_MAIN["mouse_y"]->asFloat() = event.motion.y;
            MOUSE_X = event.motion.x;
            MOUSE_Y = event.motion.y;

            if (!INFLUENCE_LIST_ENABLED)
                update_highlight();

        } else if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_ALT)) {
            // Alt key
            switch(event.key.keysym.sym) {
            case SDLK_4:
                std::cout << "Script contents:" << std::endl;
                print_raw_branch(SCRIPT_MAIN, std::cout);
                break;
            case SDLK_5:
                circa::reload_branch_from_file(SCRIPT_MAIN);
                std::cout << "Script reloaded" << std::endl;
                break;
            }
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                SCRIPT_MAIN["up_pressed"]->asBool() = true; break;
            case SDLK_DOWN:
                SCRIPT_MAIN["down_pressed"]->asBool() = true; break;
            case SDLK_LEFT:
                SCRIPT_MAIN["left_pressed"]->asBool() = true; break;
            case SDLK_RIGHT:
                SCRIPT_MAIN["right_pressed"]->asBool() = true; break;
            case SDLK_SPACE:
                SCRIPT_MAIN["space_pressed"]->asBool() = true; break;
            case SDLK_ESCAPE:
                continueMainLoop = false; break;
            case SDLK_i:
                update_highlight();
                if (HIGHLIGHT != NULL)
                    INFLUENCE_LIST_ENABLED = true;
                else
                    INFLUENCE_LIST_ENABLED = false;
                break;
            case SDLK_1:
                //toggle_influencer
                break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch(event.key.keysym.sym) {
            case SDLK_UP:
                SCRIPT_MAIN["up_pressed"]->asBool() = false; break;
            case SDLK_DOWN:
                SCRIPT_MAIN["down_pressed"]->asBool() = false; break;
            case SDLK_LEFT:
                SCRIPT_MAIN["left_pressed"]->asBool() = false; break;
            case SDLK_RIGHT:
                SCRIPT_MAIN["right_pressed"]->asBool() = false; break;
            case SDLK_SPACE:
                SCRIPT_MAIN["space_pressed"]->asBool() = false; break;
            }
        }

        try {
            SCRIPT_MAIN.eval();

            draw_highlight();
            draw_influencers();

        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            return 0;
        }

        // Update the screen
        if( SDL_Flip(SCREEN) == -1 )
            return 1;

        SDL_Delay( 10 );
    }

    // Quit SDL
    SDL_Quit();

    return 0;
}
