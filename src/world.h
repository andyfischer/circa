// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct LoadedDll;

struct World {
    circa::Value actorList;
    caStack* actorStack;

    circa::Value looseModules;

    std::map<std::string, LoadedDll*>* loadedDlls;
    std::map<Name, EvaluateFunc>* nativePatches;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
void actor_send_message(ListData* actor, caValue* message);
void actor_run_message(caStack* stack, ListData* actor, caValue* message);
ListData* find_actor(World* world, const char* name);

// A 'loose' module is one that doesn't live at the top-level, and it doesn't have a name.
Branch* create_loose_module(caWorld* world);

void refresh_all_modules(caWorld* world);

// Native function patches

// Add the name-func association to the world's table of native patches.
void add_native_patch(World* world, Name name, EvaluateFunc func);

// Lookup a native patch by name. May return NULL.
EvaluateFunc find_native_patch(World* world, Name name);

void load_native_patch_dll(World* world, const char* filename);

} // namespace circa
