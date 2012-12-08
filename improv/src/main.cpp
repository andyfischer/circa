
#include <cstdio>

#include "SDL/SDL.h"

#include "circa/circa.h"

#include "RenderTarget.h"
#include "FontBitmap.h"

caWorld* g_world;
SDL_Surface* g_surface;

bool fix_current_directory()
{
    // First step, we need to find the "ca" directory. If we're running from a Mac
    // bundle, then we'll need to walk up a few directories.

    while (true) {

        if (circa_file_exists("ca"))
            return true;

        // chdir to parent
        circa::Value currentDir, parentDir;
        circa_cwd(&currentDir);
        circa_get_parent_directory(&currentDir, &parentDir);
        // If we reached the top, then fatal.
        if (circa_equals(&currentDir, &parentDir)) {
            printf("Fatal: Couldn't find the 'ca' directory");
            return false;
        }
        
        circa_chdir(&parentDir);
    }
}

int main(int argc, char *argv[])
{
    // Initialize Circa
    g_world = circa_initialize();

    if (!fix_current_directory())
        return -1;
 
    circa_add_module_search_path(g_world, "ca");

    circa_load_module_from_file(g_world, "EngineBindings", "ca/EngineBindings.ca");
    circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");
    circa_load_module_from_file(g_world, "UserApi", "ca/UserApi.ca");
    circa_load_module_from_file(g_world, "Shell", "ca/Shell.ca");
    circa_load_module_from_file(g_world, "App", "ca/App.ca");

    RenderTarget_moduleLoad(circa_create_native_patch(g_world, "RenderTarget"));
    FontBitmap_moduleLoad(circa_create_native_patch(g_world, "FontBitmap"));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }
    
    int width = 300;
    int height = 300;

    g_surface = SDL_SetVideoMode(width, height, 32, SDL_OPENGL); 

    if (g_surface == NULL) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        exit(1);     
    }

    SDL_Event event;
    while (true) {

         //DrawScreen(screen,h++);
         if (SDL_WaitEvent(&event)) {      
              switch (event.type) 
              {
              case SDL_QUIT:
	              exit(0);
	              break;
              case SDL_KEYDOWN:
                   printf("keypress\n");
                   break;
              }
         }
    }

    return 1;
}
