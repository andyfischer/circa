// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>
#include <unistd.h>

#include "SDL/SDL.h"

#include "circa/circa.h"

#include "ResourceManager.h"
#include "RenderList.h"
#include "FontBitmap.h"

caWorld* g_world;
SDL_Surface* g_surface;

void getcwd(caValue* cwd)
{
    char buf[1024];
    if (getcwd(buf, sizeof(buf)) != NULL)
        circa_set_string(cwd, buf);
    else
        circa_set_string(cwd, "");
}

void chdir(caValue* dir)
{
    chdir(circa_string(dir));
}

bool fix_current_directory()
{
    // First step, we need to find the "ca" directory. If we're running from a Mac
    // bundle, then we might need to walk up a few directories.

    while (true) {

        if (circa_file_exists("ca"))
            return true;

        // chdir to parent
        circa::Value currentDir, parentDir;
        getcwd(&currentDir);
        circa_get_parent_directory(&currentDir, &parentDir);
        // If we reached the top, then fatal.
        if (circa_equals(&currentDir, &parentDir)) {
            printf("Fatal: Couldn't find the 'ca' directory");
            return false;
        }
        
        chdir(&parentDir);
    }
}

Uint32 redraw_timer_callback(Uint32 interval, void *not_used)
{
    SDL_Event e;
    e.type = SDL_USEREVENT;
    e.user.code = 0;
    e.user.data1 = NULL;
    e.user.data2 = NULL;
    SDL_PushEvent(& e);
    return interval;
}

int main(int argc, char *argv[])
{
    // Initialize Circa
    g_world = circa_initialize();

    if (!fix_current_directory())
        return -1;
 
    circa_add_module_search_path(g_world, "ca");

    // Load any modules that are either 1) directly accessed by C code, or 2) are patched with native code.
    circa_load_module_from_file(g_world, "RenderList", "ca/RenderList.ca");
    circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");
    circa_load_module_from_file(g_world, "UserApi", "ca/UserApi.ca");
    circa_load_module_from_file(g_world, "Shell", "ca/Shell.ca");
    circa_load_module_from_file(g_world, "App", "ca/App.ca");

    // Apply native patches.
    RenderList_moduleLoad(circa_create_native_patch(g_world, "RenderList"));
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
    
    // Setup ResourceManager
    ResourceManager resourceManager;

    // Setup RenderList
    RenderList renderList;
    renderList.setup(&resourceManager);

    caStack* stack = circa_alloc_stack(g_world);
    circa_push_module(stack, "Shell");
    
    // Initial redraw event
    redraw_timer_callback(0, NULL);

    // Event loop.
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
              case SDL_USEREVENT: {
                   // Tick & redraw.
                   circa_restart(stack);

                   caValue* msgs = circa_input(stack, 0);
                   circa_set_list(msgs, 1);
                   caValue* redrawMsg = circa_index(msgs, 0);
                   circa_set_list(redrawMsg, 2);
                   circa_set_name(circa_index(redrawMsg, 0), circa_to_name("redraw"));
                  
                   circa_make(circa_index(redrawMsg, 1), circa_find_type(g_world, "RenderList"));
                   circa_handle_set_object(circa_index(redrawMsg, 1), &renderList);

                   circa_run(stack);

                   if (circa_has_error(stack))
                       circa_print_error_to_stdout(stack);
                  
                   // Prepare another timer
                   SDL_TimerID timer_id = SDL_AddTimer(16, redraw_timer_callback, NULL);
                   if (timer_id == NULL) {
                       printf("SDL_AddTimer failed: %s\n", SDL_GetError());
                       exit(1);
                   }

                   break;
              }
              }
         }
    }

    return 1;
}

void DrawScreen()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    SDL_GL_SwapBuffers();
}
