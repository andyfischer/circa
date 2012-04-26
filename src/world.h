// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

struct caWorld {
    circa::Value actorList;
    caStack* mainStack;

protected:
    // Disallow C++ construction
    caWorld();
    ~caWorld();
};

namespace circa {

caWorld* alloc_world();

} // namespace circa
