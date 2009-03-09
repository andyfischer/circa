#include "SDL/SDL.h"
#include "SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
const float HIGHLIGHT_MIN_DIST = 5.0;

int MOUSE_X = 0;
int MOUSE_Y = 0;

SDL_Surface* SCREEN = NULL;
circa::Term* POINT_FUNC = NULL;
circa::Branch SCRIPT_MAIN;
circa::Term* HIGHLIGHT = NULL;

void load_image(circa::Term* caller)
{
    // Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;

    // The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    std::string filename = circa::as_string(caller->input(0));

    // Load the image
    loadedImage = SDL_LoadBMP( filename.c_str() );

    // If nothing went wrong in loading the image
    if( loadedImage != NULL )
    {
        // Create an optimized image
        optimizedImage = SDL_DisplayFormat( loadedImage );

        // Free the old image
        SDL_FreeSurface( loadedImage );
    }

    // Return the optimized image
    circa::as<SDL_Surface*>(caller) = optimizedImage;
}

void apply_surface(circa::Term* caller)
{
    SDL_Surface* destination = circa::as<SDL_Surface*>(caller->input(0));
    SDL_Surface* source = circa::as<SDL_Surface*>(caller->input(1));
    int x = circa::as_int(caller->input(2));
    int y = circa::as_int(caller->input(3));

    // Make a temporary rectangle to hold the offsets
    SDL_Rect offset;

    // Give the offsets to the rectangle
    offset.x = x;
    offset.y = y;

    // Blit the surface
    SDL_BlitSurface( source, NULL, destination, &offset );
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

int main( int argc, char* args[] )
{
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
    SDL_WM_SetCaption( "Hello World", NULL );

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
        } else if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
            case SDLK_4:
                std::cout << "Script contents:" << std::endl;
                print_raw_branch(SCRIPT_MAIN, std::cout);
                break;
            case SDLK_5:
                circa::reload_branch_from_file(SCRIPT_MAIN);
                std::cout << "Script reloaded" << std::endl;
                break;
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

            update_highlight();
            draw_highlight();

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

