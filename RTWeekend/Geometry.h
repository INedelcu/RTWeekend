#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "AABB.h"
#include "Ray.h"

class Material;

struct HitDesc
{
	inline void SetFaceNormal(const Ray& ray, const Vector3f& outwardNormal)
	{
		frontFace = Dot(ray.direction, outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}

	Material* material;
	Vector3f position;
	Vector3f normal;
	float u;
	float v;
	float t;
	bool frontFace;
};

class Geometry
{
public:
	virtual bool Hit(const RayDesc& rayDesc, HitDesc& hitDesc) const = 0;
	virtual void GetBoundingBox(AABB& aabb) const = 0;
};

#endif // GEOMETRY_H
