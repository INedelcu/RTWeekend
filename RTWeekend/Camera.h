#ifndef CAMERA_H
#define CAMERA_H

#include "RTWeekend.h"

class Camera
{
public:
	Camera() = default;
	Camera(
		const Vector3f& lookFrom, 
		const Vector3f& lookAt, 
		const Vector3f& up, 
		float vFov,
		float aspectRatio,
		float aperture,
		float focusDist)
	{
		float theta = DegreesToRadians(vFov);
		float h = tanf(theta / 2.0f);
		float viewportHeight = 2.0f * h;
		float viewportWidth = aspectRatio * viewportHeight;
		float focalLength = 1.0f;

		w = Normalize(lookFrom - lookAt);
		u = Normalize(Cross(up, w));
		v = Normalize(Cross(w, u));

		origin = lookFrom;
		horizontal = focusDist * viewportWidth * u;
		vertical = focusDist * viewportHeight * v;
		lowerLeftCorner = origin - vertical / 2 - horizontal / 2 - focusDist * w;
		lensRadius = aperture / 2;
	}

	Ray GetRay(float s, float t) const
	{
		Vector3f rd = lensRadius * RandomInUnitDisc();
		Vector3f offset = u * rd.x + v * rd.y;
		return Ray(origin + offset, Normalize(lowerLeftCorner + s * horizontal + t * vertical - origin - offset));
	}

private:
	Vector3f origin;
	Vector3f lowerLeftCorner;
	Vector3f horizontal;
	Vector3f vertical;
	Vector3f u, v, w;
	float lensRadius;
};

#endif
