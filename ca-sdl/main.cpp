/*This source code copyrighted by Lazy Foo' Productions (2004-2009) and may not
be redestributed without written permission.*/

//The headers
#include "SDL/SDL.h"
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

void SDL_FreeSurface_c(circa::Term* caller)
{
    SDL_Surface* surface = circa::as<SDL_Surface*>(caller->input(0));
}

int main( int argc, char* args[] )
{
    circa::initialize();

    circa::Branch branch;

    circa::import_type<SDL_Surface*>(branch, "SDL_Surface");

    circa::import_function(branch, load_image,
            "load-image(string) -> SDL_Surface");
    circa::import_function(branch, apply_surface,
            "apply-surface(SDL_Surface,SDL_Surface,int,int)");
    circa::import_function(branch, SDL_FreeSurface_c,
            "SDL_FreeSurface(SDL_Surface)");

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return 1;
    }

    //Set up the screen
    SDL_Surface* screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT,
            SCREEN_BPP, SDL_SWSURFACE );
    circa::as<SDL_Surface*>(branch.eval("screen = SDL_Surface()")) = screen;

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return 1;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Hello World", NULL );

    //Load the images
    branch.eval("message = load-image('hello_world.bmp')");
    branch.eval("background = load-image('background.bmp')");

    // Construct the redrawing branch
    circa::Branch &redraw = branch.startBranch("redraw");

    //Apply the background to the screen
    redraw.apply("apply-surface(screen, background, 0, 0)");

    circa::Int message_x(redraw, "message_x");
    circa::Int message_y(redraw, "message_y");

    message_x = 180;
    message_y = 140;

    //Apply the message to the screen
    redraw.apply("apply-surface(screen, message, message_x, message_y)");

    // Main loop
    while (true) {
        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
            break;

        redraw.eval();

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }

        SDL_Delay( 10 );
    }

    //Free the surfaces
    branch.eval("SDL_FreeSurface(message)");
    branch.eval("SDL_FreeSurface(background)");

    //Quit SDL
    SDL_Quit();

    return 0;
}

