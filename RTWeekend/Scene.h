#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>

#include "Geometry.h"

using std::shared_ptr;
using std::make_shared;

class Scene : public Geometry
{
public:
	Scene()
	{
	}

	void Add(shared_ptr<Geometry> geometry)
	{
		geometries.push_back(geometry);
	}

	void Clear()
	{
		geometries.clear();
	}

	virtual bool Hit(const RayDesc& rayDesc, HitDesc& hitDesc) const override
	{
		bool hitFound = false;

		RayDesc tempRayDesc = rayDesc;

		for (const auto& geom : geometries)
		{
			if (geom->Hit(tempRayDesc, hitDesc))
			{
				hitFound = true;
				tempRayDesc.tmax = hitDesc.t;
			}
		}

		return hitFound;
	}

public:
	std::vector<shared_ptr<Geometry>> geometries;
};


#endif // SCENE_H
