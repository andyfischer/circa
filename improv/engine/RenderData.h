// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

struct RenderList;

struct RenderData
{
    virtual void destroy() = 0;
    virtual bool destroyed() = 0;
};
