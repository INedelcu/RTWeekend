#ifndef RT_WEEKEND_H
#define RT_WEEKEND_H

// Things that made it fast:
// - custom random function for [0, 1]
// - caching radius^2 and 1 / radius of the sphere.
// - aligning Vector3f to 16 bytes using declspec

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

#include "Vector3f.h"

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535f;

inline float DegreesToRadians(float degrees)
{
	return degrees * pi / 180.0f;
}

thread_local uint32_t s_RndState = 0x5a2456fd;
static uint32_t XorShift32()
{
	uint32_t x = s_RndState;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	s_RndState = x;
	return x;
}

static float s_InvRandMax = 1.0f / (float)0xFFFFFFFF;

float RandomFloat01()
{
	return float(XorShift32()) * s_InvRandMax;
}

inline Vector3f RandomUnitVector()
{
	float z = RandomFloat01() * 2.0f - 1.0f;
	float a = RandomFloat01() * 2 * pi;
	float r = sqrtf(1.0f - z * z);
	float x = r * cosf(a);
	float y = r * sinf(a);
	return Vector3f(x, y, z);
}

inline float RandomFloat(float min, float max)
{
	return min + (max - min) * RandomFloat01();
}

inline Vector3f RandomInUnitDisc()
{
	// Ray Tracing Gems 1 - 16.5.1.1 POLAR MAPPING
	float r = sqrtf(RandomFloat01());
	float a = RandomFloat01() * 2 * pi;
	float x = r * cosf(a);
	float y = r * sinf(a);
	return Vector3f(x, y, 0);
}

inline float Clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// A ray depth of 1 means that only primary rays can intersect geometries and evaluate their materials.
const int g_MaxRayDepthSolid = 6;
const int g_MaxRayDepthTransparent = 10;

const float g_TMin = 0.001f;
const float g_TMax = 100000.0f;

#include "Color3f.h"
#include "Geometry.h"
#include "Ray.h"
#include "RayPayload.h"
#include "Scene.h"
#include "Vector3f.h"

void TraceRay(const Scene& scene, const RayDesc& rayDesc, RayPayload& payload);

#endif
