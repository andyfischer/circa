// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "improv_common.h"

#include "SDL.h"

#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iostream>

#ifdef __MACH__
  #include <mach/clock.h>
  #include <mach/mach.h>
#endif

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "cairo_surface.h"
#include "gl.h"
#include "key_codes.h"

double get_time();

static bool file_exists(const char* filename)
{
    FILE* fp = fopen(filename, "r");

    if (fp == NULL)
        return false;

    fclose(fp);
    return true;
}

static void improv_getcwd(caValue* cwd)
{
    char buf[1024];
    if (getcwd(buf, sizeof(buf)) != NULL)
        circa_set_string(cwd, buf);
    else
        circa_set_string(cwd, "");
}

bool fix_current_directory()
{
    // Find the "ca" directory. If we're running from a Mac bundle, then we might need to
    // walk up a few directories.

    while (true) {

        if (file_exists("ca"))
            return true;

        // chdir to parent
        circa::Value currentDir, parentDir;
        improv_getcwd(&currentDir);
        circa_get_parent_directory(&currentDir, &parentDir);
        // If we reached the top, then fatal.
        if (circa_equals(&currentDir, &parentDir)) {
            printf("fatal: Couldn't find the 'ca' directory");
            exit(-1);
        }
        
        chdir(circa_string(&parentDir));
    }
}

struct AudioState {
    circa::Value currentEffect;
    int effectPos;
};

AudioState* audio = NULL;

void setup_sdl_audio()
{
    audio = new AudioState();

    SDL_AudioSpec spec;
    extern void fill_audio(void *udata, Uint8 *stream, int len);

    /* Set the audio format */
    spec.freq = 44000;
    spec.format = AUDIO_S16;
    spec.channels = 2;    /* 1 = mono, 2 = stereo */
    spec.samples = 1024;  /* Good low-latency value for callback */
    spec.callback = fill_audio;
    spec.userdata = NULL;

    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&spec, NULL) < 0 ) {
        printf("fatal: Couldn't open audio: %s\n", SDL_GetError());
        exit(-1);
    }
}

void play_audio(caVM* vm)
{
    circa_move(circa_input(vm, 0), &audio->currentEffect);
    audio->effectPos = 0;
}

void fill_audio(void* udata, Uint8* stream, int numBytes)
{
    uint16_t* samples = (uint16_t*) stream;
    int sampleCount = numBytes / 4;

    if (circa_is_null(&audio->currentEffect)) {
        memset(stream, 0, numBytes);
        return;
    }

    char* effectBytes;
    uint32_t effectNumBytes;
    circa_blob_data(&audio->currentEffect, &effectBytes, &effectNumBytes);

    int remainingEffectBytes = effectNumBytes - audio->effectPos;

    if (remainingEffectBytes <= 0) {
        circa_set_null(&audio->currentEffect);
        memset(stream, 0, numBytes);
        return;
    }

    if (remainingEffectBytes > numBytes)
        remainingEffectBytes = numBytes;

    memcpy(stream, effectBytes + audio->effectPos, remainingEffectBytes);
    audio->effectPos += remainingEffectBytes;
}

double get_time()
{
    timespec time;

    #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
        clock_serv_t cclock;
        mach_timespec_t mts;
        host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
        clock_get_time(cclock, &mts);
        mach_port_deallocate(mach_task_self(), cclock);
        time.tv_sec = mts.tv_sec;
        time.tv_nsec = mts.tv_nsec;

    #else
        clock_gettime(CLOCK_REALTIME, &time);
    #endif

    return 1.0 * time.tv_sec + time.tv_nsec / 1000000000.0;
}

void WindowRelease(void* ptr)
{
}

void make_window(caVM* vm)
{
    caValue* rect = circa_input(vm, 0);

    float x, y, x2, y2;
    circa_vec4(rect, &x, &y, &x2, &y2);
    int width = x2 - x;
    int height = y2 - y;

    SDL_Window* window = SDL_CreateWindow("improv", x, y,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_GL_CreateContext(window);

    glViewport(0, 0, width, height);
    
    // Other GL setup
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    
#ifndef NACL
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
#endif

    circa_set_boxed_native_ptr(circa_output(vm), window, WindowRelease);
}

void Window__set_size(caVM* vm)
{
    //todo
}

void Window__swap(caVM* vm)
{
    SDL_Window* window = (SDL_Window*) circa_get_boxed_native_ptr(circa_input(vm, 0));
    SDL_GL_SwapWindow(window);
}

void setup_native_patches(caWorld* world)
{
    caNativePatch* improvPatch = circa_create_native_patch(world, "improv");
    circa_patch_function(improvPatch, "play_audio", play_audio);
    circa_patch_function(improvPatch, "make_window", make_window);
    circa_patch_function(improvPatch, "Window.set_size", Window__set_size);
    circa_patch_function(improvPatch, "Window.swap", Window__swap);
    circa_finish_native_patch(improvPatch);

    cairo_native_patch(circa_create_native_patch(world, "cairo"));

    caNativePatch* gl = circa_create_native_patch(world, "gl");
    gl_native_patch(gl);
    circa_finish_native_patch(gl);
}

bool sdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0) {
        printf("SDL_Init failed\n");
        return false;
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
    return true;
}

void sdl_consume_input_events(caValue* outputList)
{
    circa_set_list(outputList, 0);

    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {

        switch (sdlEvent.type) 
        {
        case SDL_QUIT: {
            exit(0);
            #if 0
            caValue* event = circa_append(outputList);
            circa_set_list(event, 1);
            circa_set_symbol(circa_index(event, 0), "quit");
            #endif
            return;
        }

        case SDL_KEYDOWN: {
            caValue* event = circa_append(outputList);
            circa_set_list(event, 2);
            circa_set_symbol(circa_index(event, 0), "down");
            sdl_key_to_symbol(sdlEvent.key.keysym.sym, circa_index(event, 1));
            break;
        }

        case SDL_KEYUP: {
            caValue* event = circa_append(outputList);
            circa_set_list(event, 2);
            circa_set_symbol(circa_index(event, 0), "up");
            sdl_key_to_symbol(sdlEvent.key.keysym.sym, circa_index(event, 1));
            break;
        }

        case SDL_MOUSEBUTTONDOWN: {
            double mouseX = sdlEvent.button.x;
            double mouseY = sdlEvent.button.y;

            caValue* event = circa_append(outputList);
            circa_set_list(event, 3);
            circa_set_symbol(circa_index(event, 0), "down");
            if (sdlEvent.button.button == SDL_BUTTON_LEFT)
                circa_set_symbol(circa_index(event, 1), "left_mouse");
            else
                circa_set_symbol(circa_index(event, 1), "right_mouse");
            circa_set_vec2(circa_index(event, 2), mouseX, mouseY);
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            double mouseX = sdlEvent.button.x;
            double mouseY = sdlEvent.button.y;

            caValue* event = circa_append(outputList);
            circa_set_list(event, 3);
            circa_set_symbol(circa_index(event, 0), "up");
            if (sdlEvent.button.button == SDL_BUTTON_LEFT)
                circa_set_symbol(circa_index(event, 1), "left_mouse");
            else
                circa_set_symbol(circa_index(event, 1), "right_mouse");
            circa_set_vec2(circa_index(event, 2), mouseX, mouseY);
            break;
        }

        case SDL_MOUSEWHEEL: {
            caValue* event = circa_append(outputList);
            circa_set_list(event, 3);
            circa_set_symbol(circa_index(event, 0), "mouse_wheel");
            circa_set_vec2(circa_index(event, 1), sdlEvent.wheel.x, sdlEvent.wheel.y);
            break;
        }

        case SDL_WINDOWEVENT: {
            switch (sdlEvent.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                caValue* event = circa_append(outputList);
                circa_set_list(event, 2);
                circa_set_symbol(circa_index(event, 0), "window_resize");
                circa_set_vec2(circa_index(event, 1),
                    sdlEvent.window.data1, sdlEvent.window.data2);
                break;
            }
            break;
        }
        }
    }
}

void sdl_get_mouse_pos(caValue* output)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    circa_set_vec2(output, mouseX, mouseY);
}

extern "C" int main(int argc, char *argv[])
{
    caWorld* world = circa_initialize();

    caValue args;
    args.set_list(0);
    for (int i=1; i < argc; i++)
        circa_set_string(circa_append(&args), argv[i]);

    fix_current_directory();

    circa_use_local_filesystem(world, "");
    circa_add_module_search_path(world, "ca");

    setup_native_patches(world);
    setup_sdl_audio();

    if (!sdl_init())
        return 1;

    if (circa_length(&args) < 1) {
        printf("Missing script argument\n");
        return 1;
    }
    
    caValue* arg = circa_index(&args, 0);

    caBlock* module = circa_load_module_by_filename(world, "ca/improv_shell.ca");
    caBlock* moduleMain = circa_find_function_local(module, "main");

    caVM* vm = circa_new_vm(world);
    circa_set_main(vm, moduleMain);

    circa_copy(&args, circa_env_insert(vm, "args"));

#ifdef NACL
    circa_set_bool(circa_env_insert(vm, "gl_es2"), true);
#endif

    // Main loop
    while (true) {
        #ifndef NACL
            circa_update_changed_files(world);
        #endif

        circa::Value inputEvents;
        sdl_consume_input_events(circa_env_insert(vm, "inputEvents"));
        sdl_get_mouse_pos(circa_env_insert(vm, "mouse"));
        circa_set_float(circa_env_insert(vm, "time"), SDL_GetTicks() / 1000.0);

        circa_run(vm);

        if (circa_has_error(vm)) {
            printf("top level error: %s\n", circa_vm_get_error(vm)->to_c_string());

            circa_dump_stack_trace(vm);
            
            #ifndef NACL
                exit(1);
            #endif
        }

        SDL_Delay(1);
    }

    return 0;
}
