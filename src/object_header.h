// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Header used for any objects that participate in GC
struct ObjectHeader
{
    char magicalHeader[6];

    Type* type;

    // If we're 'referenced', then we can only be deleted by a GC pass, not manually.
    bool referenced;

    // If we're 'root', then we can only be deleted manually, not by GC.
    bool root;

    int refcount;

    // Used during GC collection
    GCColor gcColor;

    // The object's body will be contiguous in memory.
    char body[0];

    ObjectHeader()
      : type(NULL),
        referenced(false),
        root(false),
        refcount(0),
        gcColor(0)
    {}
};

} // namespace circa
