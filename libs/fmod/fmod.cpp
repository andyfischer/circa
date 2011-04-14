
#include "fmod.h"
#include "fmod_errors.h"
#include "circa.h"
#include "importing_macros.h"

FMOD_SYSTEM *g_system = NULL;
circa::Type *g_soundType = NULL;

const int g_numChannels = 32;

void initialize()
{
    if (g_system == NULL) {
        FMOD_System_Create(&g_system);
        FMOD_System_Init(g_system, g_numChannels, FMOD_INIT_NORMAL, NULL);
    }
}

CA_FUNCTION(load_sound)
{
    initialize();

    FMOD_SOUND *sound;

    std::cout << "load_sound(" << circa::as_cstring(INPUT(0)) << ")" << std::endl;

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
    float freq = FLOAT_INPUT(1);
    FMOD_CHANNEL *channel;

    static int nextChannel = 0;

    result = FMOD_System_PlaySound(g_system, (FMOD_CHANNELINDEX) nextChannel, sound, 0, &channel);
    FMOD_Channel_SetFrequency(channel, freq);

    nextChannel++;
    if (nextChannel >= g_numChannels)
        nextChannel = 0;

    // nextChannel is a big hack because I'm having trouble getting fmod to stop running out
    // of channels.

    if (result != FMOD_OK)
        std::cout << "Error in FMOD_System_PlaySound: " << FMOD_ErrorString(result) << std::endl;
}

void setup(circa::Branch& kernel)
{
    g_soundType = unbox_type(kernel["Sound"]);
    circa::install_function(kernel["load_sound_int"], load_sound);
    circa::install_function(kernel["play_sound"], play_sound);
}
