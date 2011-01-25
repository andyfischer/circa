// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

struct PauseStatus
{
    enum Reason { NONE, USER_REQUEST, RUNTIME_ERROR };

    bool paused;
    Reason reason;

    PauseStatus() : paused(false), reason(NONE) {}
};
