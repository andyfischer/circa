// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

struct PauseStatus
{
    enum Reason { NONE, USER_REQUEST, RUNTIME_ERROR };

    bool paused;
    Reason reason;

    PauseStatus() : paused(false), reason(NONE) {}
};
