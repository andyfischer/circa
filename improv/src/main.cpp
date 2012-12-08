
#include <cstdio>

#include "circa/circa.h"

#include "RenderTarget.h"
#include "FontBitmap.h"

caWorld* g_world;

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
    g_world = circa_initialize();

    if (!fix_current_directory())
        return -1;
 
    circa_add_module_search_path(g_world, "ca");

    circa_load_module_from_file(g_world, "EngineBindings", "ca/EngineBindings.ca");
    circa_load_module_from_file(g_world, "InputEvent", "ca/InputEvent.ca");
    circa_load_module_from_file(g_world, "UserApi", "ca/UserApi.ca");

    RenderTarget_moduleLoad(circa_create_native_patch(g_world, "RenderTarget"));
    FontBitmap_moduleLoad(circa_create_native_patch(g_world, "FontBitmap"));

    return 1;
}