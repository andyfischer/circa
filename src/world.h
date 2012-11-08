// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {

    Branch* root;

    circa::Value actorList;
    caStack* actorStack;

    NativeModuleWorld* nativeModuleWorld;
    FileWatchWorld* fileWatchWorld;

    // Next unused global IDs.
    int nextTermID;
    int nextBranchID;
    int nextStackID;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* create_world();
void world_initialize(World* world);

void actor_send_message(ListData* actor, caValue* message);
void actor_run_message(caStack* stack, ListData* actor, caValue* message);
ListData* find_actor(World* world, const char* name);

void refresh_all_modules(caWorld* world);

// Load a script from the given filename, and install it in the global namespace with the
// given module name. If the module already exists, then we'll replace the existing
// contents, and we'll update any existing references that point to the replaced code.
Branch* load_script_to_global_name(World* world, const char* filename, const char* globalName);

} // namespace circa
