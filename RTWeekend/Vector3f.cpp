#include "Vector3f.h"

#include <limits>

const Vector3f Vector3f::zero(0, 0, 0);
const Vector3f Vector3f::one(1, 1, 1);
const Vector3f Vector3f::plusInf(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
const Vector3f Vector3f::minusInf(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
