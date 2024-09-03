#ifndef RAYPAYLOAD_H
#define RAYPAYLOAD_H

#include "Color3f.h"

struct RayPayload
{
    Color3f		color;
    uint32_t	rayDepth;
};

#endif
