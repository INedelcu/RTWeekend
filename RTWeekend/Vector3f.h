#ifndef VECTOR3F_H
#define VECTOR3F_H

#include <math.h>

#define FMIN(a, b) ((a < b) ? (a) : (b))
#define FMAX(a, b) ((a > b) ? (a) : (b))

class Vector3f
{
public:
	Vector3f() 
	{ 
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	Vector3f(float x, float y, float z) 
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3f& operator=(const Vector3f& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3f operator-() const 
	{
		return Vector3f(-x, -y, -z);
	}

	Vector3f& operator+=(const Vector3f& v) 
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3f& operator-=(const Vector3f& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3f& operator*=(const Vector3f& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3f& operator*=(float t)
	{
		x *= t;
		y *= t;
		z *= t;
		return *this;
	}

	Vector3f& operator/=(float t)
	{
		return *this *= 1 / t;
	}

	float LengthSquared() const
	{
		return x * x + y * y + z * z;
	}

	float Length() const
	{
		return sqrtf(LengthSquared());
	}

	bool NearZero() const
	{
		const float epsilon = 1e-8f;
		return (fabsf(x) < epsilon) && (fabsf(y) < epsilon) && (fabsf(z) < epsilon);
	}

public:
	float x;
	float y;
	float z;

	static const Vector3f one;
	static const Vector3f zero;
	static const Vector3f plusInf;
	static const Vector3f minusInf;
};

using Color3f = Vector3f;

inline Vector3f operator+(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector3f operator-(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3f operator*(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline Vector3f operator*(float t, const Vector3f& v)
{
	return Vector3f(t * v.x, t * v.y, t * v.z);
}

inline Vector3f operator*(const Vector3f& v, float t)
{
	return t * v;
}

inline Vector3f operator/(const Vector3f& v, float t)
{
	return (1 / t) * v;
}

inline Vector3f operator/(float t, const Vector3f& v)
{
	return Vector3f(t / v.x, t / v.y, t / v.z);
}

inline float Dot(const Vector3f& a, const Vector3f& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3f Cross(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline Vector3f Reflect(const Vector3f& i, const Vector3f& n)
{
	return i - 2 * Dot(i, n) * n;
}

inline Vector3f Refract(const Vector3f& i, const Vector3f& n, float iorRatio)
{
	float cosTheta = -Dot(i, n);
	Vector3f tPerp = iorRatio * (i + cosTheta * n);
	Vector3f rParallel = -sqrtf(fabsf(1 - tPerp.LengthSquared())) * n;
	return tPerp + rParallel;
}

inline Vector3f Min(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(FMIN(a.x, b.x), FMIN(a.y, b.y), FMIN(a.z, b.z));
}

inline Vector3f Max(const Vector3f& a, const Vector3f& b)
{
	return Vector3f(FMAX(a.x, b.x), FMAX(a.y, b.y), FMAX(a.z, b.z));
}

inline float MinComponent(const Vector3f& a)
{
	return FMIN(a.x, FMIN(a.y, a.z));
}

inline float MaxComponent(const Vector3f& a)
{
	return FMAX(a.x, FMAX(a.y, a.z));
}

inline Vector3f Normalize(const Vector3f& v)
{
	return v / v.Length();
}


#endif