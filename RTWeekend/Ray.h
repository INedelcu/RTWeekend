#ifndef RAY_H
#define RAY_H

#include "Vector3f.h"

class AABB;
class Scene;

class Ray
{
public:
	friend class AABB;
	friend class Scene;

	Ray() {};
	Ray(const Vector3f& o, const Vector3f& d) : origin(o), direction(d)
	{
	}

	Vector3f At(float t) const { return origin + t * direction; }

private:
	void ComputeSign()
	{
		sign[0] = invDirection.x < 0;
		sign[1] = invDirection.y < 0;
		sign[2] = invDirection.z < 0;
	}

	void CacheHitHelpers()
	{
		invDirection = 1.0f / direction;
		ComputeSign();
	}

public:
	Vector3f	origin;
	Vector3f	direction;

private:
	Vector3f	invDirection;
	int			sign[3];
};

struct RayDesc
{
	Ray ray;
	float tmin;
	float tmax;
};

#endif // RAY_H
