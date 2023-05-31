#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>

#include "BVH.h"
#include "Geometry.h"

using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;

class Scene
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

	bool Hit(const RayDesc& rayDesc, HitDesc& hitDesc) const
	{
#define USE_BVH 1
		
#if USE_BVH
		if (geometries.empty())
			return false;

		// Hit calls update the tmax with the closest hit found during traversal.
		RayDesc tempRayDesc = rayDesc;

		return root->Hit(tempRayDesc, hitDesc);
#else
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
#endif
	}

	void BuildAccelerationStructure()
	{
		root = make_unique<BVHNode>(geometries, 0, geometries.size());
	}

public:
	unique_ptr<BVHNode>					root;
	std::vector<shared_ptr<Geometry>>	geometries;
};


#endif // SCENE_H
