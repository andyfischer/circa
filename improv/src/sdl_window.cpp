// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "improv_common.h"

#include "SDL2/SDL.h"

#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <iostream>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#include "cairo_surface.h"
#include "key_codes.h"

namespace improv {

struct ImprovWindow {

    SDL_Window* _sdl_window;
    SDL_Surface* _sdl_screen;
    
    float _width;
    float _height;
    caValue* _inputEvents;
    
    float _mouseX;
    float _mouseY;
    
    double _elapsedTime;

    ImprovWindow() :
        _sdl_screen(NULL)
    {}

    void initOpenGL();
    void mainLoop();
    void setSize(float w, float h);
};

void ImprovWindow::mainLoop()
{
    // Main loop.
    while (true) {
        
        // Handle events.
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent)) {

            switch (sdlEvent.type) 
            {
            case SDL_QUIT:
                printf("received SDL_QUIT\n");
                return;

            case SDL_KEYDOWN: {

                #ifndef NACL
                    // Quit on Escape key
                    if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                        return;

                    #if 0
                    // Fullscreen on command-F
                    if (sdlEvent.key.keysym.sym == SDLK_f && (sdlEvent.key.keysym.mod & KMOD_CTRL))
                        toggleFullscreen();
                    #endif
                #endif

                caValue* event = circa_append(_inputEvents);
                circa_set_list(event, 2);
                circa_set_symbol(circa_index(event, 0), "down");
                sdl_key_to_symbol(sdlEvent.key.keysym.sym, circa_index(event, 1));
                break;
            }
            case SDL_KEYUP: {
                caValue* event = circa_append(_inputEvents);
                circa_set_list(event, 2);
                circa_set_symbol(circa_index(event, 0), "up");
                sdl_key_to_symbol(sdlEvent.key.keysym.sym, circa_index(event, 1));
                break;
            }

            case SDL_MOUSEBUTTONDOWN: {
                double mouseX = sdlEvent.button.x;
                double mouseY = sdlEvent.button.y;

                caValue* event = circa_append(_inputEvents);
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

                caValue* event = circa_append(_inputEvents);
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
                caValue* event = circa_append(_inputEvents);
                circa_set_list(event, 3);
                circa_set_symbol(circa_index(event, 0), "mouse_wheel");
                circa_set_vec2(circa_index(event, 1), sdlEvent.wheel.x, sdlEvent.wheel.y);
                break;
            }

            case SDL_WINDOWEVENT: {
                switch (sdlEvent.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    int width = sdlEvent.window.data1;
                    int height = sdlEvent.window.data2;
                    setSize(width, height);
                    break;
                }
                break;
            }
            }
        }

        // Done handling events. Redraw screen.

        _elapsedTime = SDL_GetTicks() / 1000.0;

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        _mouseX = mouseX;
        _mouseY = mouseY;

        redraw();

        SDL_GL_SwapWindow(_sdl_window);
        SDL_Delay(1);
    }
}
    
void ImprovWindow::setSize(float w, float h)
{
    _width = w;
    _height = h;
    
    glViewport(0, 0, _width, _height);
    
    printf(buf, "glViewport %d %d", _width, _height);
    
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
}
    
void ImprovWindow::redraw()
{

#ifndef NACL
    circa_update_changed_files(m_world);
#endif
    
    if (m_windowWidth == 0 && m_windowHeight == 0) {
        log("error: App::redraw() called before setSize()");
        return;
    }
    
    double startTime = get_time();
    
    glClear( GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/ );
    
    circa_update_changed_files(m_world);
    
    circa_copy(m_inputEvents, circa_env_insert(m_stack, "inputEvents"));
    
    circa_set_vec2(circa_env_insert(m_stack, "mouse"),
                   m_context.mouseX, m_context.mouseY);
    circa_set_vec2(circa_env_insert(m_stack, "windowSize"),
                   m_windowWidth, m_windowHeight);
    circa_set_float(circa_env_insert(m_stack, "time"), m_context.elapsedTime);
    circa_set_float(circa_env_insert(m_stack, "lastFrameDuration"), m_context.lastFrameDuration);
    
#ifdef NACL
    circa_set_bool(circa_env_insert(m_stack, "gl_es2"), true);
#endif
    
    circa_run(m_stack);
    
    if (circa_has_error(m_stack)) {
        circa_dump_stack_trace(m_stack);
        
#ifndef NACL
        exit(1);
#endif
    }
    
    // Cleanup stack
    circa_restart(m_stack);
    
    // Cleanup
    circa_set_list(m_inputEvents, 0);
    
    m_context.lastFrameDuration = get_time() - startTime;
    
#define DUMP_PERF_STATS 0
#if DUMP_PERF_STATS
    printf("frame duration = %f\n", (float) m_context.lastFrameDuration);
    circa_perf_stats_dump();
    circa_perf_stats_reset();
#endif

}

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

void play_audio(caStack* stack)
{
    circa_move(circa_input(stack, 0), &audio->currentEffect);
    audio->effectPos = 0;
}

void fill_audio(void *udata, Uint8 *stream, int numBytes)
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

extern "C" int main(int argc, char *argv[])
{
    if (argc <= 1) {
        printf("Missing script argument\n");
        return 1;
    }
    
    caWorld* world = circa_initialize();

    fix_current_directory();

    circa_use_local_filesystem(world, "");

    caNativePatch* improvPatch = circa_create_native_patch(world, "improv");
    circa_patch_function(improvPatch, "play_audio", play_audio);
    circa_finish_native_patch(improvPatch);

    setup_sdl_audio();

    improv::App app;
    app.m_world = world;

    app.start();

    const char* arg = argv[1];
    circa_set_string(circa_env_insert(app.m_stack, "scriptName"), arg);

    ImprovWindow window(&app);

    // Initialize SDL surface.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0) {
        return 1;
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );

    const int width = 1000;
    const int height = 600;

    window._sdl_window = SDL_CreateWindow("improv", 50, 50,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_GL_CreateContext(window._sdl_window);

    app.setSize(width, height);

    SDL_PauseAudio(0);

    window.mainLoop();
    return 0;
}


} // namespace improv
