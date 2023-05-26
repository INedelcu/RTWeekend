#ifndef MATERIAL_H
#define MATERIAL_H

#include "RTWeekend.h"

struct HitDesc;
struct RayDesc;
struct RayPayload;
class Scene;

class Material
{
public:
	virtual void ClosestHitShader(const Scene& scene, const RayDesc& rayDesc, const HitDesc& hitDesc, RayPayload& payload) const = 0;
};

#endif