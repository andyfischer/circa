/*This source code copyrighted by Lazy Foo' Productions (2004-2009) and may not
be redestributed without written permission.*/

//The headers
#include "SDL/SDL.h"
#include "SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

SDL_Surface* SCREEN = NULL;

void load_image(circa::Term* caller)
{
    //Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    std::string filename = circa::as_string(caller->input(0));

    //Load the image
    loadedImage = SDL_LoadBMP( filename.c_str() );

    //If nothing went wrong in loading the image
    if( loadedImage != NULL )
    {
        //Create an optimized image
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old image
        SDL_FreeSurface( loadedImage );
    }

    //Return the optimized image
    circa::as<SDL_Surface*>(caller) = optimizedImage;
}

void apply_surface(circa::Term* caller)
{
    SDL_Surface* destination = circa::as<SDL_Surface*>(caller->input(0));
    SDL_Surface* source = circa::as<SDL_Surface*>(caller->input(1));
    int x = circa::as_int(caller->input(2));
    int y = circa::as_int(caller->input(3));

    //Make a temporary rectangle to hold the offsets
    SDL_Rect offset;

    //Give the offsets to the rectangle
    offset.x = x;
    offset.y = y;

    //Blit the surface
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
        as_int(caller->input(4)));
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

void SDL_FreeSurface_c(circa::Term* caller)
{
    SDL_Surface* surface = circa::as<SDL_Surface*>(caller->input(0));
}

int main( int argc, char* args[] )
{
    circa::initialize();

    circa::import_type<SDL_Surface*>(*circa::KERNEL, "SDL_Surface");

    circa::import_function(*circa::KERNEL, load_image,
            "load-image(string) -> SDL_Surface");
    circa::import_function(*circa::KERNEL, apply_surface,
            "apply-surface(SDL_Surface,SDL_Surface,int,int)");
    circa::import_function(*circa::KERNEL, SDL_FreeSurface_c,
            "SDL_FreeSurface(SDL_Surface)");
    circa::import_function(*circa::KERNEL, line_to,
            "line_to(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, rectangle,
            "rectangle(float,float,float,float,int)");
    circa::import_function(*circa::KERNEL, fill_rectangle,
            "fill_rectangle(float,float,float,float,int)");

    //Set up the screen
    SCREEN = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT,
            SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( SCREEN == NULL )
    {
        return 1;
    }

    std::string filename;
    if (argc > 1)
        filename = args[1];
    else
        filename = "cgame/main.ca";

    std::cout << "loading file: " << filename << std::endl;

    circa::Branch script_main;
    circa::evaluate_file(script_main, filename);

    circa::print_raw_branch(script_main, std::cout);

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return 1;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Hello World", NULL );

    // Access the redraw() function
    circa::Branch &redraw = circa::get_subroutine_branch(script_main["redraw"]);

    circa::Float mouse_x(script_main, "mouse_x");
    circa::Float mouse_y(script_main, "mouse_y");
    circa::Int rect_size(redraw, "rect_size");
    circa::Bool up_pressed(script_main, "up_pressed");
    circa::Bool down_pressed(script_main, "down_pressed");
    circa::Bool left_pressed(script_main, "left_pressed");
    circa::Bool right_pressed(script_main, "right_pressed");

    // Main loop
    while (true) {
        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
            break;

        if (event.type == SDL_MOUSEMOTION) {
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
        } else if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
            case SDLK_5:
                circa::reload_branch_from_file(script_main);
                std::cout << "Script reloaded, new contents:" << std::endl;
                print_raw_branch(script_main, std::cout);
                break;
            case SDLK_UP:
                up_pressed = true; break;
            case SDLK_DOWN:
                down_pressed = true; break;
            case SDLK_LEFT:
                left_pressed = true; break;
            case SDLK_RIGHT:
                right_pressed = true; break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch(event.key.keysym.sym) {
            case SDLK_UP:
                up_pressed = false; break;
            case SDLK_DOWN:
                down_pressed = false; break;
            case SDLK_LEFT:
                left_pressed = false; break;
            case SDLK_RIGHT:
                right_pressed = false; break;
            }
        }

        try {
            redraw.eval();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            return 0;
        }

        //Update the screen
        if( SDL_Flip(SCREEN) == -1 )
        {
            return 1;
        }

        SDL_Delay( 10 );
    }

    //Free the surfaces
    script_main.eval("SDL_FreeSurface(background)");

    //Quit SDL
    SDL_Quit();

    return 0;
}

