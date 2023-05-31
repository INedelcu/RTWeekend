#ifndef RAY_H
#define RAY_H

#include "Vector3f.h"

class Ray
{
public:
	Ray() = default;
	Ray(const Vector3f& o, const Vector3f& d) : origin(o), direction(d) {}

	Vector3f At(float t) const { return origin + t * direction; }

public:
	Vector3f origin;
	Vector3f direction;
};

struct RayDesc
{
	Ray ray;
	float tmin;
	float tmax;
};

#endif // RAY_H
