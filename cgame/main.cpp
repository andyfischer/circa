/*This source code copyrighted by Lazy Foo' Productions (2004-2009) and may not
be redestributed without written permission.*/

//The headers
#include "SDL/SDL.h"
#include "/opt/local/include/SDL/SDL_gfxPrimitives.h"

#include <string>

#include "circa.h"

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

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
    rectangleColor(circa::as<SDL_Surface*>(caller->input(0)),
        (int) as_float(caller->input(1)),
        (int) as_float(caller->input(2)),
        (int) as_float(caller->input(3)),
        (int) as_float(caller->input(4)),
        as_int(caller->input(5)));
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
    circa::import_function(*circa::KERNEL, rectangle,
            "rectangle(SDL_Surface,float,float,float,float,int)");

    //Set up the screen
    SDL_Surface* screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT,
            SCREEN_BPP, SDL_SWSURFACE );

    circa::import_value(*circa::KERNEL, "SDL_Surface", &screen, "screen");

    circa::Branch branch;
    circa::evaluate_file(branch, "cgame/main.ca");

    circa::print_raw_branch(branch, std::cout);

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return 1;
    }

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return 1;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Hello World", NULL );

    //Load the images

    // Make a call to redraw()
    circa::Branch &redraw = circa::get_subroutine_branch(branch["redraw"]);
    std::cout << "redraw:" << std::endl;
    circa::print_raw_branch(redraw, std::cout);

    circa::Float mouse_x(redraw, "mouse_x");
    circa::Float mouse_y(redraw, "mouse_y");
    circa::Int rect_size(redraw, "rect_size");

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
            if (event.key.keysym.sym == SDLK_5) {
                circa::reload_branch_from_file(branch);
            }
        }

        redraw.eval();

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }

        SDL_Delay( 10 );
    }

    //Free the surfaces
    branch.eval("SDL_FreeSurface(background)");

    //Quit SDL
    SDL_Quit();

    return 0;
}

