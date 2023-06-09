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

	static void GetUVs(const Vector3f& p, float& u, float& v) {
		// p: a given point on the sphere of radius one, centered at the origin.
		// u: returned value [0,1] of angle around the Y axis from X=-1.
		// v: returned value [0,1] of angle from Y=-1 to Y=+1.
		//     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
		//     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
		//     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

		auto theta = acosf(-p.y);
		auto phi = atan2f(-p.z, p.x) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
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
		GetUVs(outwardNormal, hitDesc.u, hitDesc.v);
		hitDesc.material = material.get();

		return true;
	}

	virtual void GetBoundingBox(AABB& aabb) const override
	{
		aabb.min = center - Vector3f(radius, radius, radius);
		aabb.max = center + Vector3f(radius, radius, radius);
	}

public:
	Vector3f center;
	float radius;
	float invRadius;
	float radius2;
	shared_ptr<Material> material;
};

#endif