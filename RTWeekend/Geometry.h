#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Ray.h"

class Material;

struct HitDesc
{
	inline void SetFaceNormal(const Ray& ray, const Vector3f& outwardNormal)
	{
		frontFace = Dot(ray.direction, outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}

	shared_ptr<Material> material;
	Vector3f position;
	Vector3f normal;
	float t;
	bool frontFace;
};

struct RayDesc
{
	Ray ray;
	float tmin;
	float tmax;
};

class Geometry
{
public:
	virtual bool Hit(const RayDesc& rayDesc, HitDesc& hitDesc) const = 0;
};

#endif // GEOMETRY_H
