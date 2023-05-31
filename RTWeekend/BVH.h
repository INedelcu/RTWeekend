#ifndef BVH_H
#define BVH_H

#include "Geometry.h"
#include "Scene.h"

inline bool BoxCompare(const shared_ptr<Geometry> a, const shared_ptr<Geometry> b, int axis)
{
	AABB boxA;
	AABB boxB;

	a->GetBoundingBox(boxA);
	b->GetBoundingBox(boxB);
	
	float* minAvec = &boxA.min.x;
	float* minBvec = &boxB.min.x;

	return minAvec[axis] < minBvec[axis];
}

bool BoxCompareX(const shared_ptr<Geometry> a, const shared_ptr<Geometry> b)
{
	return BoxCompare(a, b, 0);
}

bool BoxCompareY(const shared_ptr<Geometry> a, const shared_ptr<Geometry> b)
{
	return BoxCompare(a, b, 1);
}

bool BoxCompareZ(const shared_ptr<Geometry> a, const shared_ptr<Geometry> b)
{
	return BoxCompare(a, b, 2);
}

static int s_BVHNodeCount = 0;

class BVHNode
{
public:
	struct Data
	{
		AABB					aabb;
		shared_ptr<Geometry>	geometry;	// Valid only for leaf nodes.
	};

	inline int RandomInt(int min, int max) 
	{
		// Returns a random integer in [min,max].
		return static_cast<int>(RandomFloat((float)min, (float)(max + 1)));
	}

	BVHNode(std::vector<shared_ptr<Geometry>>& geometries, size_t start, size_t end)
	{
		s_BVHNodeCount++;

		if (geometries.size() == 0)
			return;

		auto tempGeometries = geometries; // Create a modifiable array of the source scene objects

		int axis = RandomInt(0, 2);

		auto comparator = (axis == 0) ? BoxCompareX
			: (axis == 1) ? BoxCompareY
			: BoxCompareZ;

		size_t count = end - start;
		if (count == 1)
		{
			data.geometry = tempGeometries[start];
			if (data.geometry)
			{
				data.geometry->GetBoundingBox(data.aabb);
			}
			return;
		}
		else if (count == 2)
		{
			if (comparator(tempGeometries[start], tempGeometries[start + 1]))
			{
				left = make_unique<BVHNode>(tempGeometries, start, start + 1);
				right = make_unique<BVHNode>(tempGeometries, start + 1, start + 2);
			}
			else
			{
				right = make_unique<BVHNode>(tempGeometries, start, start + 1);
				left = make_unique<BVHNode>(tempGeometries, start + 1, start + 2);
			}
		}
		else
		{
			std::sort(tempGeometries.begin() + start, tempGeometries.begin() + end, comparator);

			size_t mid = start + count / 2;
			left = make_unique<BVHNode>(tempGeometries, start, mid);
			right = make_unique<BVHNode>(tempGeometries, mid, end);
		}


		AABB boxLeft, boxRight;

		if (left)
		{
			if (left->data.geometry)
			{
				left->data.geometry->GetBoundingBox(boxLeft);
			}
			else
			{
				boxLeft = left->data.aabb;
			}
		}

		if (right)
		{
			if (right->data.geometry)
			{
				right->data.geometry->GetBoundingBox(boxRight);
			}
			else
			{
				boxRight = right->data.aabb;
			}
		}
		
		boxLeft.Encapsulate(boxRight);
		data.aabb = boxLeft;
	}

	~BVHNode()
	{
		s_BVHNodeCount--;
	}

	inline bool Hit(RayDesc& rayDesc, HitDesc& hitDesc) const
	{
		if (data.aabb.Hit(rayDesc))
		{
			if (data.geometry)
			{
				// Leaf node - check geometry intersection.
				bool hit = data.geometry->Hit(rayDesc, hitDesc);
				if (hit)
				{
					rayDesc.tmax = hitDesc.t;
				}
				return hit;
			}
			else
			{
				bool hitLeft = left->Hit(rayDesc, hitDesc);
				bool hitRight = right->Hit(rayDesc, hitDesc);

				return hitLeft || hitRight;
			}
		}
		else
			return false;
	}

	inline void Clear()
	{
		if (left)
			left->Clear();

		if (right)
			right->Clear();

		left.reset();
		right.reset();
	}

public:
	unique_ptr<BVHNode> left;
	unique_ptr<BVHNode> right;
	Data				data;
};

#endif