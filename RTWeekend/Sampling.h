#ifndef SAMPLING_H
#define SAMPLING_H

#include "RTWeekend.h"
#include "Vector3f.h"

// Z points up (should be aligned with the normal).
Vector3f CosineWeightedSample(float s, float t)
{
	t = std::min(t, 0.9999f);
	float u = 2 * pi * s;
	float v = sqrtf(1 - t);
	return Vector3f(v * cosf(u), v * sinf(u), sqrtf(t));
}

void FrisvadONB(const Vector3f& n, Vector3f& b1, Vector3f& b2)
{
	if (n.z < -0.99999f) // Handle the singularity
	{
		b1 = Vector3f(0.0f, -1.0f, 0.0f);
		b2 = Vector3f(-1.0f, 0.0f, 0.0f);
		return;
	}
	const float a = 1.0f / (1.0f + n.z);
	const float b = -n.x * n.y * a;
	b1 = Vector3f(1.0f - n.x * n.x * a, b, -n.x);
	b2 = Vector3f(b, 1.0f - n.y * n.y * a, -n.y);
}

#endif