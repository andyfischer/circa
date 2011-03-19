
#include "fmod.h"
#include "fmod_errors.h"
#include "circa.h"
#include "importing_macros.h"

FMOD_SYSTEM *g_system = NULL;
circa::Type *g_soundType = NULL;

void initialize()
{
    if (g_system == NULL) {
        FMOD_System_Create(&g_system);
        FMOD_System_Init(g_system, 32, FMOD_INIT_NORMAL, NULL);
    }
}

CA_FUNCTION(load_sound)
{
    initialize();

    FMOD_SOUND *sound;
    FMOD_RESULT result = FMOD_System_CreateSound(g_system, circa::as_cstring(INPUT(0)),
            FMOD_SOFTWARE, 0, &sound);

    if (result != FMOD_OK) {
        circa::error_occurred(CONTEXT, CALLER, FMOD_ErrorString(result));
        circa::set_null(OUTPUT);
    } else {
        circa::set_pointer(OUTPUT, g_soundType, sound);
    }
}

CA_FUNCTION(play_sound)
{
    initialize();

    FMOD_RESULT result;

    FMOD_SOUND *sound = (FMOD_SOUND*) INPUT(0)->value_data.ptr;

    FMOD_CHANNEL *channel;
    result = FMOD_System_PlaySound(g_system, FMOD_CHANNEL_FREE, sound, 0, &channel);
}

void setup(circa::Branch& kernel)
{
    g_soundType = unbox_type(kernel["Sound"]);
    circa::install_function(kernel["load_sound_int"], load_sound);
    circa::install_function(kernel["play_sound"], play_sound);
}
