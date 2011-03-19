
#include "fmod.h"
#include "circa.h"

FMOD_SYSTEM *g_system = NULL;


void initialize()
{
    if (g_system == NULL) {
        FMOD_System_Create(&g_system);
        FMOD_System_Init(g_system, 32, FMOD_INIT_NORMAL, NULL);
    }
}

CA_FUNCTION(load_sound)
{
}

CA_FUNCTION(play_sound)
{
}

void setup(circa::Branch& kernel)
{
    circa::import_function(kernel, load_sound, "load_sound(string filename) -> Sound");
    circa::import_function(kernel, play_sound, "play_sound(Sound sound)");
}
