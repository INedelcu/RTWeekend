#ifndef SAMPLING_H
#define SAMPLING_H

#include "RTWeekend.h"
#include "Vector3f.h"

// Z points up (should be aligned with the normal).
Vector3f CosineWeightedSample(float s, float t)
{
	float u = 2 * pi * s;
	float v = sqrtf(1 - t);
	return Vector3f(v * cosf(u), v * sinf(u), sqrtf(t));
}


void FrisvadONB(const Vector3f& n, Vector3f& b1, Vector3f& b2)
{
    float k = 1.0f / std::max<float>(1.0f + n.z, 0.00001f);
    const float a = n.y * k;
    const float b = n.y * a;
    const float c = -n.x * a;
    b1 = Vector3f(n.z + b, c, -n.x);
    b2 = Vector3f(c, 1.0f - b, -n.y);
}

#endif