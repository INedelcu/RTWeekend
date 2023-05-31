#ifndef AABB_H
#define AABB_H

#include "Ray.h"

#include <emmintrin.h>

class AABB
{
public:
	AABB() {}
	AABB(const Vector3f& min, const Vector3f& max)
	{
		this->min = min;
		this->max = max;
	}

	void Encapsulate(const AABB& aabb)
	{
		min = Min(min, aabb.min);
		max = Max(max, aabb.max);
	}

	bool Hit(const RayDesc& rayDesc) const
	{
		// A Ray-Box Intersection Algorithm and Efficient Dynamic Voxel Rendering, Majercik et al.
		Vector3f invRayDir = 1.0f / rayDesc.ray.direction;
		Vector3f t0 = (min - rayDesc.ray.origin) * invRayDir;
		Vector3f t1 = (max - rayDesc.ray.origin) * invRayDir;
		Vector3f tminv = Min(t0, t1);
		Vector3f tmaxv = Max(t0, t1);
		float tmin = FMAX(MaxComponent(tminv), rayDesc.tmin);
		float tmax = FMIN(MinComponent(tmaxv), rayDesc.tmax);
		return (tmin <= tmax);
	}

public:
	Vector3f min;
	Vector3f max;

};

#endif
