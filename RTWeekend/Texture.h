#ifndef TEXTURE_H
#define TEXTURE_H

#include "RTWeekend.h"

class Texture 
{
public:
	virtual Color3f Sample(float u, float v) const = 0;
};

class SolidColorTexture : public Texture 
{
public:
	SolidColorTexture() {}
	SolidColorTexture(const Color3f& c) : color(c) {}
	
    virtual Color3f Sample(float u, float v) const override
    {
        return color;
    }

private:
	Color3f color;
};

#endif // TEXTURE_H
