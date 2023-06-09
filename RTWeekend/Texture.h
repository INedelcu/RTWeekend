#ifndef TEXTURE_H
#define TEXTURE_H

#include "RTWeekend.h"

class Texture 
{
public:
	virtual Color3f Sample(float u, float v, const Vector3f& p) const = 0;
};

class SolidColorTexture : public Texture 
{
public:
	SolidColorTexture() {}
	SolidColorTexture(const Color3f& c) : color(c) {}
	
	virtual Color3f Sample(float u, float v, const Vector3f& p) const override
	{
		return color;
	}

private:
	Color3f color;
};

class CheckerTexture : public Texture 
{
public:
	CheckerTexture() {}

	CheckerTexture(shared_ptr<Texture> _even, shared_ptr<Texture> _odd)
		: even(_even)
		, odd(_odd) 
	{}

	CheckerTexture(const Color3f& c1, const Color3f& c2)
		: even(make_shared<SolidColorTexture>(c1))
		, odd(make_shared<SolidColorTexture>(c2)) 
	{}

	virtual Color3f Sample(float u, float v, const Vector3f& p) const override
	{
		auto sines = sin(10 * p.x) * sin(10 * p.y) * sin(10 * p.z);
		if (sines < 0)
			return odd->Sample(u, v, p);
		else
			return even->Sample(u, v, p);
	}

public:
	shared_ptr<Texture> odd;
	shared_ptr<Texture> even;
};

#endif // TEXTURE_H
