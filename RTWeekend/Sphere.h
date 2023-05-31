#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"
#include "Vector3f.h"
#include "Material.h"

class Sphere : public Geometry
{
public:
	Sphere() : center(), radius(1.0f), invRadius(1.0f), radius2(1.0f) {}
	Sphere(shared_ptr<Material> material, const Vector3f& center, float radius)
	{
		this->center = center;
		this->radius = radius;
		this->material = material;

		radius2 = radius * radius;
		invRadius = 1.0f / radius;
	}

	virtual bool Hit(const RayDesc& rayDesc, HitDesc& hitDesc) const override
	{
		Vector3f oc = rayDesc.ray.origin - center;
		float a = rayDesc.ray.direction.LengthSquared();
		float halfb = Dot(oc, rayDesc.ray.direction);
		float c = oc.LengthSquared() - radius2;
		float delta = halfb * halfb - a * c;

		if (delta < 0)
			return false;

		float sqrtDelta = sqrtf(delta);

		float root = (-halfb - sqrtDelta) / a;
		if (root < rayDesc.tmin || rayDesc.tmax < root)
		{
			root = (-halfb + sqrtDelta) / a;
			if (root < rayDesc.tmin || rayDesc.tmax < root)
				return false;
		}

		hitDesc.t = root;
		hitDesc.position = rayDesc.ray.At(root);
		Vector3f outwardNormal = (hitDesc.position - center) * invRadius;				
		hitDesc.SetFaceNormal(rayDesc.ray, outwardNormal);
		hitDesc.material = material;

		return true;
	}

	virtual bool GetBoundingBox(AABB& aabb) const override
	{
		aabb.min = center - Vector3f(radius, radius, radius);
		aabb.max = center + Vector3f(radius, radius, radius);
		return true;
	}

public:
	Vector3f center;
	float radius;
	float invRadius;
	float radius2;
	shared_ptr<Material> material;
};

#endif